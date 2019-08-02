#include "variable.h"
#include "dimension.h"
#include "group.h"
#include <type_traits>
#include <algorithm>

namespace fminc4
{

template <typename T>
nc_var<T>::nc_var(std::shared_ptr<nc_file> theFile, int theNcId, int theVarId) : itsNcFile(theFile), itsNcId(theNcId), itsVarId(theVarId)
{
}

template <typename T>
void nc_var<T>::Write(const std::vector<T>& vals)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(itsNcFile->fileWriteMutex);

        int status = nc_put_var(itsNcId, itsVarId, vals.data());
	if(status != NC_NOERR)
		throw status;
	nc_sync(itsNcId);
}

template <typename T>
void nc_var<T>::Write(const std::vector<T>& vals, const std::vector<size_t>& start,const std::vector<size_t>& count)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(itsNcFile->fileWriteMutex);

        int status = nc_put_vara(itsNcId, itsVarId, start.data(), count.data(), vals.data());
	if(status != NC_NOERR)
		throw status;
	nc_sync(itsNcId);
}

template <typename T>
void nc_var<T>::Write(T value, const std::vector<size_t>& index)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(itsNcFile->fileWriteMutex);

	int status = nc_put_var1(itsNcId, itsVarId, index.data(), &value);
        if(status != NC_NOERR)
		throw status;
	nc_sync(itsNcId);
}

template <typename T>
std::vector<T> nc_var<T>::Read()
{
        nc_sync(itsNcId);

	size_t size = 1;
	for(auto x : GetDims())
		size *= x.Size();

	std::vector<T> ret(size);

	int status = nc_get_var(itsNcId, itsVarId, ret.data());
        if(status != NC_NOERR)
                throw status;

	return ret;
}

template <typename T>
T nc_var<T>::Read(const std::vector<size_t>& index)
{
	// thread safety required?
	nc_sync(itsNcId);

        T ret;
        int status = nc_get_var1(itsNcId, itsVarId, index.data(), &ret);
	if(status != NC_NOERR)
		throw status;
        return ret;
}

template <typename T>
std::vector<T> nc_var<T>::Read(const std::vector<size_t>& start,const std::vector<size_t>& count)
{
	// thread safety required?
	nc_sync(itsNcId);

        std::vector<T> ret(std::accumulate(count.begin(), count.end(), 1.0, std::multiplies<T>()));
        int status = nc_get_vara(itsNcId, itsVarId, start.data(), count.data(), ret.data());
	if(status != NC_NOERR)
		throw status;
        return ret;
}

// Attributes
template <typename T>
std::vector<std::string> nc_var<T>::ListAtts() const
{
        int natts;

        nc_inq_natts(itsNcId, &natts);
	nc_inq_var(itsNcId,itsVarId,NULL,NULL,NULL,NULL,&natts);

        std::vector<std::string> ret;

        for (int i = 0; i<natts; ++i)
        {
                char recname[NC_MAX_NAME+1];
                int status = nc_inq_attname(itsNcId, itsVarId, i, recname);
                ret.push_back(recname);
        }

        return ret;
}

// Dimensions
template <typename T>
std::vector<nc_dim> nc_var<T>::GetDims()
{
        nc_sync(itsNcId);

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
		ret.emplace_back(itsNcFile,itsNcId,x);
	return ret;
}

template class nc_var<int>;
template class nc_var<unsigned int>;
template class nc_var<double> ;
template class nc_var<float>;
template class nc_var<short>;
template class nc_var<unsigned short>;
template class nc_var<long>;
template class nc_var<unsigned long>;
template class nc_var<long long>;
template class nc_var<unsigned long long>;
template class nc_var<signed char>;
template class nc_var<unsigned char>;

} // end namespace
