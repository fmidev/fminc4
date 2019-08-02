#ifndef DIMENSION_H
#define DIMENSION_H

#include "common.h"
#include <iostream>
#include "fminc4.h"
#include <memory>

namespace fminc4
{

class nc_dim
{
        public:
	nc_dim() = default;
	nc_dim(std::shared_ptr<nc_file>, int, int);

        std::string Name();
        void Name(const std::string&);

        size_t Size();

	int NcId();
	int DimId();

        private:
	std::shared_ptr<nc_file> itsFile;
        int itsNcId;
        int itsDimId;
};

}  // end namespace fminc4

#endif /* COMMON_H */
