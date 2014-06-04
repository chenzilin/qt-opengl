#include "videowidgetsurface.h"
#include <QtWidgets>
#include <qabstractvideosurface.h>
#include <qvideosurfaceformat.h>

VideoWidgetSurface::VideoWidgetSurface(QObject *parent)
    : QAbstractVideoSurface(parent),
      imageFormat(QImage::Format_Invalid),
      m_counter(0)
{
}

QList<QVideoFrame::PixelFormat> VideoWidgetSurface::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle)
    {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_ARGB32_Premultiplied
                << QVideoFrame::Format_RGB565
                << QVideoFrame::Format_RGB555;
    }
    else
    {
        return QList<QVideoFrame::PixelFormat>();
    }
}

bool VideoWidgetSurface::isFormatSupported(const QVideoSurfaceFormat &format) const
{
    const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    const QSize size = format.frameSize();

    return imageFormat != QImage::Format_Invalid
            && !size.isEmpty()
            && format.handleType() == QAbstractVideoBuffer::NoHandle;
}

bool VideoWidgetSurface::start(const QVideoSurfaceFormat &format)
{
    const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    const QSize size = format.frameSize();

    if (imageFormat != QImage::Format_Invalid && !size.isEmpty())
    {
        this->imageFormat = imageFormat;
        imageSize = size;
        QAbstractVideoSurface::start(format);
        return true;
    }
    else
    {
        return false;
    }
}

void VideoWidgetSurface::stop()
{
    currentFrame = QVideoFrame();
    QAbstractVideoSurface::stop();
}

bool VideoWidgetSurface::present(const QVideoFrame &frame)
{
    if (surfaceFormat().pixelFormat() != frame.pixelFormat()
            || surfaceFormat().frameSize() != frame.size())
    {
        setError(IncorrectFormatError);
        stop();

        return false;
    }
    else
    {
        currentFrame = frame;
        currentFrame.map(QAbstractVideoBuffer::ReadOnly);
        QImage image(currentFrame.bits(),
                     currentFrame.width(),
                     currentFrame.height(),
                     imageFormat);
        emit aviImage(image);
        currentFrame.unmap();
        return true;
    }
}
