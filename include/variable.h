#ifndef VARIABLE_H
#define VARIABLE_H

#include "common.h"
#include <iostream>
#include <vector>
#include "fminc4.h"
#include <memory>

namespace fminc4
{

extern std::mutex netcdfLibMutex;

template<typename T>
class nc_var
{
	// Some trick to allow template initialization of function in template class when class und function template type differ...
	template<typename ATT_TYPE> struct type { };

        public:
	nc_var() = default;
	nc_var(int, int);

	// Write data to variable
        void Write(const std::vector<T>&); // Linear chunk of data in memory, size fill the entire variable. DON'T USE WITH UNLIMITED DIMENSION VARIABLES
        void Write(const std::vector<T>&, const std::vector<size_t>&, const std::vector<size_t>&); // Data is written to a subarray defined by starting indices and length in each dimension
	void Write(T, const std::vector<size_t>&);
	//---

	// Read data from variable
	std::vector<T> Read(); // Entire variable
        T Read(const std::vector<size_t>&); // Single value from index in N-dimensional index
        std::vector<T> Read(const std::vector<size_t>&, const std::vector<size_t>&); // Read subarray defined by starting indices and length in each dimension to linear memory
	//---

	// Attributes
        template<class ATT_TYPE>
        std::vector<ATT_TYPE> GetAtt(const std::string& name)
	{
		return GetAtt(type<ATT_TYPE>(), name);
	}

	template<typename ATT_TYPE>
	void AddAtt(const std::string&, const std::vector<ATT_TYPE>&);

	template<typename ATT_TYPE>
	void AddAtt(const std::string&, const ATT_TYPE&);

	void AddTextAtt(const std::string&, const std::string&);

	std::vector<std::string> ListAtts() const;
	// end attributes

	// Dimensions
	std::vector<nc_dim> GetDims();

        private:
	template<typename ATT_TYPE>
	std::vector<ATT_TYPE> GetAtt(type<ATT_TYPE>,const std::string&);
	std::vector<std::string> GetAtt(type<std::string>,const std::string&);

	int itsNcId;
        int itsVarId;
};

//Implementation in header as otherwise all permutations of T and ATT_TYPE needed to be explicitly instantiated
template <typename T>
template <typename ATT_TYPE>
void nc_var<T>::AddAtt(const std::string& name, const std::vector<ATT_TYPE>& values)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

	int status;

        if(std::is_same<ATT_TYPE, double>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_DOUBLE, values.size(), values.data());
        }
        else if(std::is_same<ATT_TYPE, std::string>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_STRING, values.size(), values.data());
        }
        else if(std::is_same<ATT_TYPE, float>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_FLOAT, values.size(), values.data());
        }
        else if(std::is_same<ATT_TYPE, int>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_INT, values.size(), values.data());
        }
        else if(std::is_same<ATT_TYPE, long long>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_INT64, values.size(), values.data());
        }
        else
        {
                std::cout << "attribute: unknown type\n";
        }

        if(status != NC_NOERR)
                throw status;
}

//Implementation in header as otherwise all permutations of T and ATT_TYPE needed to be explicitly instantiated
template <typename T>
template <typename ATT_TYPE>
void nc_var<T>::AddAtt(const std::string& name, const ATT_TYPE& value)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        int status = NC_NOERR;

        if(std::is_same<ATT_TYPE, double>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_DOUBLE, 1, &value);
        }
        else
        {
                std::cout << "attribute: unknown type\n";
        }

        if(status != NC_NOERR)
	{
                throw status;
	}
}


template <typename T>
template <typename ATT_TYPE>
std::vector<ATT_TYPE> nc_var<T>::GetAtt(type<ATT_TYPE>,const std::string& name)
{
        // thread safety required?
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        size_t attlen;
        nc_inq_attlen(itsNcId, itsVarId, name.c_str(), &attlen);

        std::vector<ATT_TYPE> ret(attlen);

        nc_get_att(itsNcId, itsVarId, name.c_str(), ret.data());

        return ret;
}

template <typename T>
std::vector<std::string> nc_var<T>::GetAtt(type<std::string>,const std::string& name)
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
        }
}

template <typename T>
void nc_var<T>::AddTextAtt(const std::string& attName, const std::string& attValue)
{
	std::lock_guard<std::mutex> lock(netcdfLibMutex);

	int status = nc_put_att_text(itsNcId, itsVarId, attName.c_str(), attValue.length(),attValue.c_str());
	if(status != NC_NOERR)
                throw status;
}

} // end namespace fminc4
#endif /* VARIABLE_H */
