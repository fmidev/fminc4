#include "dimension.h"
#include "group.h"

namespace fminc4
{

extern std::mutex netcdfLibMutex;

nc_dim::nc_dim(int theNcId, int theDimId) : itsNcId(theNcId), itsDimId(theDimId) {}

std::string nc_dim::Name()
{
        char recname[NC_MAX_NAME+1];
        nc_inq_dimname(itsNcId, itsDimId, recname);
        return std::string(recname);
}

void nc_dim::Name(const std::string& theName)
{
	std::lock_guard<std::mutex> lock(netcdfLibMutex);
        nc_rename_dim(itsNcId, itsDimId, theName.c_str());
}

size_t nc_dim::Size()
{
        size_t dimSize;
        nc_inq_dimlen(itsNcId, itsDimId, &dimSize);
	return dimSize;
}

int nc_dim::NcId()
{
	return itsNcId;
}

int nc_dim::DimId()
{
	return itsDimId;
}

} // end namespace
