#ifndef GROUP_H
#define GROUP_H

#include "common.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <map>
#include <memory>
#include "fminc4.h"

namespace fminc4
{

class nc_group
{
        public:
        nc_group() = default;
	nc_group(std::shared_ptr<nc_file>, int);

	// sub groups
	nc_group GetGroup(const std::string&);
	void AddGroup(const std::string&);
	std::vector<std::string> ListGroups() const;
	//---

        // dimensions
        nc_dim GetDim(const std::string&);
        void AddDim(const std::string&, size_t);
	std::vector<nc_dim> ListDims() const;
	//---

	// variables
        template<typename T>
        nc_var<T> GetVar(const std::string&);

	template<typename T>
	void AddVar(const std::string&, const std::vector<nc_dim>&);

	std::vector<std::string> ListVars() const;
	//---

	// attributes
	template<typename ATT_TYPE>
	std::vector<ATT_TYPE> GetAtt(const std::string&);

	template<typename ATT_TYPE>
	void AddAtt(const std::string&, const std::vector<ATT_TYPE>&);

	std::vector<std::string> ListAtts() const;
	//---

        private:
        std::shared_ptr<nc_file> itsFile;
	int itsGroupId;
};

} // end namespace fminc4
#endif /* GROUP_H */
