#ifndef FMINC4_H
#define FMINC4_H

#include <mutex>
#include <netcdf.h>
#include "common.h"

namespace fminc4
{

struct nc_file
{
        nc_file(int theNcId) : itsNcId(theNcId) {};
        ~nc_file()
        {
                nc_close(itsNcId);
        }

        const int itsNcId;
        std::mutex fileWriteMutex;
};

nc_group Create(const std::string&);
nc_group Open(const std::string&);
bool Close(const std::string&);
void Finalize();

} // end namespace fminc4

#endif // FMINC4_H
