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

#ifndef _HPP_OBJECTFACTORY
#define _HPP_OBJECTFACTORY

#include "../defines.hpp"

#include "object.hpp"

#include "../types/singleton.hpp"

namespace blunted {

  class ObjectFactory : public Singleton<ObjectFactory> {

    public:
      ObjectFactory();
      virtual ~ObjectFactory();

      virtual void Exit();

      boost::intrusive_ptr<Object> CreateObject(const std::string &name,
                                                e_ObjectType objectType,
                                                std::string postfix = "_copy");
      boost::intrusive_ptr<Object> CopyObject(boost::intrusive_ptr<Object> source, std::string postfix = "_copy");
      void RegisterPrototype(e_ObjectType objectType, boost::intrusive_ptr<Object> prototype);

    protected:
      std::map <e_ObjectType, boost::intrusive_ptr<Object> > prototypes;

  };

}

#endif
