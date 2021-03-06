#include "group.h"
#include "variable.h"
#include "dimension.h"

namespace fminc4
{
extern std::mutex netcdfLibMutex;

nc_group::nc_group(std::shared_ptr<nc_file> theFile, int theGroupId) : itsFile(theFile), itsGroupId(theGroupId)
{
}

// Dimensions
nc_dim nc_group::GetDim(const std::string& theName)
{
	std::lock_guard<std::mutex> lock(netcdfLibMutex);

	int itsDimId;
        int status = nc_inq_dimid(itsGroupId, theName.c_str(), &itsDimId);
        if (status != NC_NOERR)
                throw status;

        return nc_dim(itsGroupId,itsDimId);
}

nc_dim nc_group::AddDim(const std::string& theName, size_t theSize)
{
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        int dimId;
        int status = nc_def_dim(itsGroupId, theName.c_str(), theSize, &dimId);
	if (status != NC_NOERR)
		throw status;

	return nc_dim(itsGroupId,dimId);
}

std::vector<nc_dim> nc_group::ListDims() const
{
	std::lock_guard<std::mutex> lock(netcdfLibMutex);

	int ndims;

	nc_inq_ndims(itsGroupId, &ndims);
	int dimids[ndims];

	int status = nc_inq_dimids(itsGroupId, &ndims, dimids, 0);
	if (status != NC_NOERR)
		throw status;

	std::vector<nc_dim> ret;

	for (int i = 0; i<ndims; ++i)
	{
		ret.emplace_back(itsGroupId,dimids[i]);
	}

	return ret;
}
// ---

// Variables
nc_var nc_group::GetVar(const std::string& theName)
{
	std::lock_guard<std::mutex> lock(netcdfLibMutex);

	int itsVarId;
        nc_inq_varid(itsGroupId, theName.c_str(), &itsVarId);      

	return nc_var(itsGroupId, itsVarId);
}

nc_var nc_group::AddVar(const std::string& theName, const std::vector<nc_dim>& theDims, const nc_type& theType)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        std::vector<int> itsDimIds;
        itsDimIds.reserve(theDims.size());
        for(nc_dim dim : theDims)
                itsDimIds.push_back(dim.DimId());

	int itsVarId;

        int status = nc_def_var(itsGroupId, theName.c_str(), theType, itsDimIds.size(), itsDimIds.data(), &itsVarId);
        if(status != NC_NOERR)
            throw status;

	return nc_var{itsGroupId,itsVarId};
}

std::vector<nc_var> nc_group::ListVars() const
{

	std::lock_guard<std::mutex> lock(netcdfLibMutex);
        int nvars;

        nc_inq_nvars(itsGroupId, &nvars);
        int varids[nvars];

        int status = nc_inq_varids(itsGroupId, &nvars, varids);

        std::vector<nc_var> ret;

        for (int i = 0; i<nvars; ++i)
        {
                ret.emplace_back(itsGroupId, varids[i]);
        }

        return ret;
}
// ---

// Attributes
template <typename ATT_TYPE>
std::vector<ATT_TYPE> nc_group::GetAtt(const std::string& name)
{
        // thread safety required?
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        size_t attlen;
        nc_inq_attlen(itsGroupId, NC_GLOBAL, name.c_str(), &attlen);
        std::vector<ATT_TYPE> ret(attlen);

        nc_get_att(itsGroupId, NC_GLOBAL, name.c_str(), ret.data());

        return ret;
}
template std::vector<double> nc_group::GetAtt<double>(const std::string&);

template <>
std::vector<std::string> nc_group::GetAtt(const std::string& name)
{
	std::lock_guard<std::mutex> lock(netcdfLibMutex);

        nc_type type;
        int status = nc_inq_atttype(itsGroupId, NC_GLOBAL, name.c_str(), &type);
        switch(type)
        {
                case NC_CHAR :  {
                                        size_t attlen;
                                        int status = nc_inq_attlen(itsGroupId, NC_GLOBAL, name.c_str(), &attlen);
                                        char att[attlen];
                                        status = nc_get_att(itsGroupId, NC_GLOBAL, name.c_str(), att);
                                        return std::vector<std::string>{std::string(att)};
                                }
                case NC_STRING :{
                                        std::vector<char*> chars = GetAtt<char*>(name);
                                        std::vector<std::string> ret;
                                        for(auto x : chars)
                                        {
                                                ret.emplace_back(x);
                                        }
                                        return ret;
                                }
        }
}
template std::vector<std::string> nc_group::GetAtt<std::string>(const std::string&);

template <typename ATT_TYPE>
void nc_group::AddAtt(const std::string& name, const std::vector<ATT_TYPE>& values)
{
	//...
}

std::vector<std::tuple<std::string, nc_type, size_t>> nc_group::ListAtts() const
{
	std::lock_guard<std::mutex> lock(netcdfLibMutex);

        int natts;
	nc_inq_natts(itsGroupId, &natts);

        std::vector<std::tuple<std::string, nc_type, size_t>> ret;

        for (int i = 0; i<natts; ++i)
        {
                char recname[NC_MAX_NAME+1];
		nc_type type;
		size_t size;

                nc_inq_attname(itsGroupId, NC_GLOBAL, i, recname);
		nc_inq_att(itsGroupId, NC_GLOBAL, recname, &type, &size);
                ret.emplace_back(recname, type, size);
        }

        return ret;
}
//---
} // end namespace
