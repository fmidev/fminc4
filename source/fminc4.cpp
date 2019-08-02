#include "fminc4.h"
#include <map>
#include <memory>
#include "group.h"

namespace fminc4
{

// definitions
std::mutex fileCacheMutex;
std::map<std::string, std::shared_ptr<nc_file>> fileCache;

/*
 * Create a new netcdf file
 * If file with similar name already exists in cache, return that
 * Or should this operation just fail instead?
 */

nc_group Create(const std::string& path)
{
	// Ensure thread safety
	std::lock_guard<std::mutex> lock(fileCacheMutex);

	if(fileCache.count(path) == 0)
	{
        	int itsNcId;
      		int status = nc_create(path.c_str(), NC_NETCDF4, &itsNcId);
		if(status != NC_NOERR)
			throw status;
	  	fileCache[path] = std::make_shared<nc_file>(itsNcId);
	}
	return nc_group(fileCache[path], fileCache[path]->itsNcId);
}

/*
 * Open file in read-write mode
 */

nc_group Open(const std::string& path)
{
	// Ensure thread safety
	std::lock_guard<std::mutex> lock(fileCacheMutex);

	if(fileCache.count(path) == 0)
	{
		int itsNcId;
		int status = nc_open(path.c_str(), kNcShare, &itsNcId);
        	if(status != NC_NOERR)
                	throw status;
        	fileCache[path] = std::make_shared<nc_file>(itsNcId);
	}

	return nc_group(fileCache[path], fileCache[path]->itsNcId);
}

/*
 * Attempt to close the file if there are no active object instances pointing to this file.
 * Alternatively this could force closing the file. Instances relying on this file will throw exception then.
 * I don't know which way is better...
 */

bool Close(const std::string& path)
{
        // Ensure thread safety
        std::lock_guard<std::mutex> lock(fileCacheMutex);

        if(fileCache[path].use_count() == 1)
	{
                fileCache.erase(path);
		return true;
	}

	return false;
}

void Finalize()
{
        // Ensure thread safety
        std::lock_guard<std::mutex> lock(fileCacheMutex);

        fileCache.clear();
}

} // end namespace
