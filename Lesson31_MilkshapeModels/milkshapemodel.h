#ifndef MILKSHAPEMODEL_H
#define MILKSHAPEMODEL_H

#include <QFile>
#include <QDebug>
#include "model.h"

class MilkshapeModel : public Model
{
public:
    /*	Constructor. */
    MilkshapeModel();
    /*	Destructor. */
    virtual ~MilkshapeModel();
    /*
        Load the model data into the private variables.
            filename			Model filename
    */
    virtual bool loadModelData(char *filename);
};

#endif // MILKSHAPEMODEL_H
