#ifndef COMMON_H
#define COMMON_H

#include <netcdf.h>

namespace fminc4
{

enum NcFileMode
{
	kNcReadOnly = NC_NOWRITE,
	kNcReadWrite = NC_WRITE,
	kNcShare = NC_NETCDF4|NC_SHARE|NC_WRITE,
	kNc4 = NC_NETCDF4
};

//predeclarations
class nc_group;
class nc_dim;
class nc_var;

}
#endif /* COMMON_H */
