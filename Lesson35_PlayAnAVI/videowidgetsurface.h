#ifndef VIDEOWIDGETSURFACE_H
#define VIDEOWIDGETSURFACE_H

#include <QAbstractVideoSurface>
#include <QImage>
#include <QRect>
#include <QVideoFrame>

class VideoWidgetSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    VideoWidgetSurface(QObject *parent = 0);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    bool isFormatSupported(const QVideoSurfaceFormat &format) const;

    bool start(const QVideoSurfaceFormat &format);
    void stop();

    bool present(const QVideoFrame &frame);
signals:
    void aviImage(QImage image);
private:
    QImage::Format imageFormat;
    QSize imageSize;
    QVideoFrame currentFrame;
    int m_counter;
};
#endif
