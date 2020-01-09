#include "variable.h"
#include "dimension.h"
#include "group.h"
#include <type_traits>
#include <algorithm>

namespace fminc4
{

nc_var::nc_var(int theNcId, int theVarId) : itsNcId(theNcId), itsVarId(theVarId)
{
}

nc_type nc_var::Type() const
{
	nc_type varType;
	nc_inq_vartype(itsNcId,itsVarId,&varType);
	return varType;
}

template <typename T>
void nc_var::Write(const std::vector<T>& vals)
{
        // ensure thread safety
        std::lock_guard<std::mutex> liblock(netcdfLibMutex);

        int status = nc_put_var(itsNcId, itsVarId, vals.data());
	if(status != NC_NOERR)
		throw status;
}
template void nc_var::Write<float>(const std::vector<float>&);
template void nc_var::Write<double>(const std::vector<double>&);
template void nc_var::Write<short>(const std::vector<short>&);
template void nc_var::Write<int>(const std::vector<int>&);
template void nc_var::Write<uint64_t>(const std::vector<uint64_t>&);

template <typename T>
void nc_var::Write(const std::vector<T>& vals, const std::vector<size_t>& start, const std::vector<size_t>& count)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        int status = nc_put_vara(itsNcId, itsVarId, start.data(), count.data(), vals.data());
	if(status != NC_NOERR)
		throw status;
}
template void nc_var::Write<float>(const std::vector<float>&, const std::vector<size_t>&, const std::vector<size_t>&);
template void nc_var::Write<double>(const std::vector<double>&, const std::vector<size_t>&, const std::vector<size_t>&);
template void nc_var::Write<short>(const std::vector<short>&, const std::vector<size_t>&, const std::vector<size_t>&);
template void nc_var::Write<int>(const std::vector<int>&, const std::vector<size_t>&, const std::vector<size_t>&);
template void nc_var::Write<uint64_t>(const std::vector<uint64_t>&, const std::vector<size_t>&, const std::vector<size_t>&);

template <typename T>
void nc_var::Write(T value, const std::vector<size_t>& index)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

	int status = nc_put_var1(itsNcId, itsVarId, index.data(), &value);
        if(status != NC_NOERR)
		throw status;
}
template void nc_var::Write<float>(float, const std::vector<size_t>&);
template void nc_var::Write<double>(double, const std::vector<size_t>&);
template void nc_var::Write<short>(short, const std::vector<size_t>&);
template void nc_var::Write<int>(int, const std::vector<size_t>&);
template void nc_var::Write<uint64_t>(uint64_t, const std::vector<size_t>&);

template <typename T>
std::vector<T> nc_var::Read()
{
	size_t size = 1;
	for(auto x : GetDims())
		size *= x.Size();

	std::vector<T> ret(size);

	int status = nc_get_var(itsNcId, itsVarId, ret.data());
        if(status != NC_NOERR)
                throw status;

	return ret;
}
template std::vector<float> nc_var::Read<float>();
template std::vector<double> nc_var::Read<double>();
template std::vector<short> nc_var::Read<short>();
template std::vector<int> nc_var::Read<int>();
template std::vector<uint64_t> nc_var::Read<uint64_t>();

template <typename T>
T nc_var::Read(const std::vector<size_t>& index)
{
	// thread safety required?

        T ret;
        int status = nc_get_var1(itsNcId, itsVarId, index.data(), &ret);
	if(status != NC_NOERR)
		throw status;
        return ret;
}
template float nc_var::Read<float>(const std::vector<size_t>&);
template double nc_var::Read<double>(const std::vector<size_t>&);
template short nc_var::Read<short>(const std::vector<size_t>&);
template int nc_var::Read<int>(const std::vector<size_t>&);
template uint64_t nc_var::Read<uint64_t>(const std::vector<size_t>&);

template <typename T>
std::vector<T> nc_var::Read(const std::vector<size_t>& start, const std::vector<size_t>& count)
{
	// thread safety required?

        std::vector<T> ret(std::accumulate(count.begin(), count.end(), 1.0, std::multiplies<T>()));
        int status = nc_get_vara(itsNcId, itsVarId, start.data(), count.data(), ret.data());
	if(status != NC_NOERR)
		throw status;
        return ret;
}
template std::vector<float> nc_var::Read<float>(const std::vector<size_t>&, const std::vector<size_t>&);
template std::vector<double> nc_var::Read<double>(const std::vector<size_t>&, const std::vector<size_t>&);
template std::vector<short> nc_var::Read<short>(const std::vector<size_t>&, const std::vector<size_t>&);
template std::vector<int> nc_var::Read<int>(const std::vector<size_t>&, const std::vector<size_t>&);
template std::vector<uint64_t> nc_var::Read<uint64_t>(const std::vector<size_t>&, const std::vector<size_t>&);

// Attributes
std::vector<std::tuple<std::string, nc_type, size_t>> nc_var::ListAtts() const
{
        int natts;

        nc_inq_natts(itsNcId, &natts);
	nc_inq_var(itsNcId,itsVarId,NULL,NULL,NULL,NULL,&natts);

        std::vector<std::tuple<std::string, nc_type, size_t>> ret;

        for (int i = 0; i<natts; ++i)
        {
                char recname[NC_MAX_NAME+1];
		nc_type type;
		size_t size;
		
                nc_inq_attname(itsNcId, itsVarId, i, recname);
		nc_inq_att(itsNcId, itsVarId, recname, &type, &size) ;	
                ret.emplace_back(recname, type, size);
        }

        return ret;
}

void nc_var::AddTextAtt(const std::string& attName, const std::string& attValue)
{
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        int status = nc_put_att_text(itsNcId, itsVarId, attName.c_str(), attValue.length(),attValue.c_str());
        if(status != NC_NOERR)
                throw status;
}

template <typename T>
std::vector<T> nc_var::GetAtt(const std::string& name)
{
        // thread safety required?
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        size_t attlen;
        nc_inq_attlen(itsNcId, itsVarId, name.c_str(), &attlen);

        std::vector<T> ret(attlen);

        nc_get_att(itsNcId, itsVarId, name.c_str(), ret.data());

        return ret;
}
template std::vector<double> nc_var::GetAtt<double>(const std::string&);

template <>
std::vector<std::string> nc_var::GetAtt(const std::string& name)
{
        std::lock_guard<std::mutex> lock(netcdfLibMutex);
        nc_type theType;
        int status = nc_inq_atttype(itsNcId, itsVarId, name.c_str(), &theType);
        switch(theType)
        {
                case NC_CHAR :  {
                                        size_t attlen;
                                        status = nc_inq_attlen(itsNcId, itsVarId, name.c_str(), &attlen);
                                        char att[attlen];
                                        status = nc_get_att(itsNcId, itsVarId, name.c_str(), att);
                                        return std::vector<std::string>{std::string(att).substr(0,attlen)};
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
                default:
                        throw status;
        }
}
template std::vector<std::string> nc_var::GetAtt<std::string>(const std::string&);

// Dimensions
std::vector<nc_dim> nc_var::GetDims()
{

        int ndims;
        int status = nc_inq_varndims(itsNcId, itsVarId, &ndims);
        if(status != NC_NOERR)
                throw status;

        std::vector<int> dimids(ndims);

        status = nc_inq_vardimid(itsNcId, itsVarId, dimids.data());
        if(status != NC_NOERR)
                throw status;

	std::vector<nc_dim> ret;
	ret.reserve(ndims);

        for(auto x : dimids)
		ret.emplace_back(itsNcId,x);
	return ret;
}

} // end namespace
