// Copyright 2019 Google LLC & Bastiaan Konings
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#ifndef _HPP_MANAGERS_RESOURCE
#define _HPP_MANAGERS_RESOURCE

#include "../defines.hpp"

#include "../base/log.hpp"
#include "../base/utils.hpp"

#include "../types/loader.hpp"

#include "../managers/environmentmanager.hpp"

namespace blunted {

  template <typename T>
  class Resource;

  template <typename T>
  class ResourceManager {

    public:
      ResourceManager(const std::string &typeDescription) : typeDescription(typeDescription) {};

      ~ResourceManager() {

        resources.clear();

        loaders.clear();
      };

      void RegisterLoader(const std::string &extension, Loader<T> *loader) {
        //printf("registering loader for extension %s\n", extension.c_str());
        loaders.insert(std::make_pair(extension, loader));
      }

      boost::intrusive_ptr < Resource<T> > Fetch(const std::string &filename, bool load = true, bool useExisting = true) {
        bool foo = false;
        return Fetch(filename, load, foo, useExisting);
      }

      boost::intrusive_ptr < Resource<T> > Fetch(const std::string &filename, bool load, bool &alreadyThere, bool useExisting) {
        std::string adaptedFilename = get_file_name(filename);

        // resource already loaded?

        bool success = false;
        boost::intrusive_ptr < Resource<T> > foundResource;

        if (useExisting) {
          foundResource = Find(adaptedFilename, success);
        }

        if (success) {
          // resource is already there! w00t, that'll win us some cycles
          // (or user wants a new copy)

          alreadyThere = true;

          return foundResource;
        }

        else {

          // create resource

          alreadyThere = false;
          boost::intrusive_ptr < Resource <T> > resource(new Resource<T>(adaptedFilename));

          // try to load

          if (load) {
            std::string extension = get_file_extension(filename);
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            typename std::map < std::string, Loader<T>* >::iterator iter = loaders.find(extension);
            if (iter != loaders.end()) {
              (*iter).second->Load(filename, resource);
            } else {
              Log(e_FatalError, "ResourceManager<>", "Load", "There is no loader for " + filename);
            }
          }
          Register(resource);
          return resource;
        }
      }

      boost::intrusive_ptr < Resource<T> > FetchCopy(const std::string &filename, const std::string &newName, bool &alreadyThere) {
        boost::intrusive_ptr < Resource<T> > resourceCopy;
        if (resources.find(newName) != resources.end()) {
          //Log(e_Warning, "ResourceManager", "FetchCopy", "Duplicate key '" + newName + "' - returning existing resource instead of copy (maybe just use Fetch() instead?)");
          resourceCopy = Fetch(newName, false, true);
        } else {
          boost::intrusive_ptr < Resource<T> > resource = Fetch(filename, true, alreadyThere, true);

          resourceCopy = boost::intrusive_ptr < Resource<T> >(new Resource<T>(*resource, newName));

          Register(resourceCopy);
        }

        return resourceCopy;
      }

      void RemoveUnused() {
        // periodically execute this cleanup code somewhere
        // currently invoked from scheduler, could be a user task?
        // as if it were a service..
        // would be slower, but somewhat cooler :p

        // cleanup



        typename std::map < std::string, boost::intrusive_ptr< Resource<T> > >::iterator resIter = resources.begin();
        while (resIter != resources.end()) {
          if (resIter->second->GetRefCount() == 1) {
            //printf("removing unused %s resource '%s'\n", typeDescription.c_str(), resIter->second->GetIdentString().c_str());
            resources.erase(resIter++);
          } else {
            ++resIter;
          }
        }


      }

      void Remove(const std::string &identString) {

        auto iter = resources.find(identString);
        if (iter != resources.end()) {
          resources.erase(iter);
        }

      }

    protected:

      boost::intrusive_ptr < Resource<T> > Find(const std::string &identString, bool &success) {

        typename std::map < std::string, boost::intrusive_ptr< Resource<T> > >::iterator resIter = resources.find(identString);
        if (resIter != resources.end()) {
          success = true;
          boost::intrusive_ptr < Resource<T> > resource = (*resIter).second;

          return resource;
        } else {
          success = false;

          return boost::intrusive_ptr < Resource<T> >();
        }
      }

      void Register(boost::intrusive_ptr < Resource<T> > resource) {



        //printf("registering %s\n", resource->GetIdentString().c_str());
        if (resources.find(resource->GetIdentString()) != resources.end()) {
           Remove(resource->GetIdentString());
          if (resources.find(resource->GetIdentString()) != resources.end()) {
            Log(e_FatalError, "ResourceManager", "Register", "Duplicate key '" + resource->GetIdentString() + "'");
          }
        }
        resources.insert(std::make_pair(resource->GetIdentString(), resource));
      }

      std::map < std::string, Loader<T>* > loaders;

      std::map < std::string, boost::intrusive_ptr < Resource <T> > > resources;

      std::string typeDescription;

    private:

  };

}

#endif
