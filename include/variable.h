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

class nc_var
{
	// Some trick to allow template initialization of function in template class when class und function template type differ...
	template<typename ATT_TYPE> struct type { };

        public:
	nc_var() = default;
	nc_var(int, int);

	nc_type Type() const;

	// Write data to variable
	template<typename T>
        void Write(const std::vector<T>&); // Linear chunk of data in memory, size fill the entire variable. DON'T USE WITH UNLIMITED DIMENSION VARIABLES

	template<typename T>
        void Write(const std::vector<T>&, const std::vector<size_t>&, const std::vector<size_t>&); // Data is written to a subarray defined by starting indices and length in each dimension

	template<typename T>
	void Write(T, const std::vector<size_t>&);
	//---

	// Read data from variable
	template<typename T>
	std::vector<T> Read(); // Entire variable

	template<typename T>
        T Read(const std::vector<size_t>&); // Single value from index in N-dimensional index

	template<typename T>
        std::vector<T> Read(const std::vector<size_t>&, const std::vector<size_t>&); // Read subarray defined by starting indices and length in each dimension to linear memory
	//---

	// Attributes
        template<class T>
        std::vector<T> GetAtt(const std::string& name);

	template<typename T>
	void AddAtt(const std::string&, const std::vector<T>&);

	template<typename T>
	void AddAtt(const std::string&, const T&);

	void AddTextAtt(const std::string&, const std::string&);

	// return a list of attributes linked to the variable together with the attributes type and length
	std::vector<std::tuple<std::string, nc_type, size_t>> ListAtts() const;

	//

	// Dimensions
	std::vector<nc_dim> GetDims();

        private:

	int itsNcId;
        int itsVarId;
};

//Implementation in header as otherwise all permutations of T and ATT_TYPE needed to be explicitly instantiated
template <typename T>
void nc_var::AddAtt(const std::string& name, const T& value)
{
        // ensure thread safety
        std::lock_guard<std::mutex> lock(netcdfLibMutex);

        int status = NC_NOERR;

        if(std::is_same<T, double>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_DOUBLE, 1, &value);
        }
        else if(std::is_same<T, float>::value)
        {
                status = nc_put_att(itsNcId, itsVarId, name.c_str(), NC_FLOAT, 1, &value);
        }
        else
        {
                std::cout << name << " attribute: unknown type\n";
        }

        if(status != NC_NOERR)
	{
                throw status;
	}
}

} // end namespace fminc4
#endif /* VARIABLE_H */
