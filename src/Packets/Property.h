/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <cstdarg>

#include "String.h"
#include "../TypeDefs.h"
#include "../Config.h"

namespace espMqttClientTypes {

struct PropertyItem {
  uint8_t propertyId;
  union {
    uint8_t byteCode;
    uint16_t twoByteCode;
    uint32_t fourByteCode;
    char* utf8String;
    struct {
      char* dataKey;
      char* dataValue;
    } utf8StringPair;
    struct {
      uint16_t length;
      uint8_t* data;
    } binaryData;
  } data;
};

class PropertyCollection {
 public:
  PropertyCollection();

  // initialize with number of properties as capacity
  explicit PropertyCollection(size_t capacity);

  template<typename ... Args>
  PropertyCollection(const Args... args)
  : _properties(nullptr)
  , _capacity(NUMBER_PROPERTIES)
  , _length(0) {
    _properties = reinterpret_cast<PropertyItem*>(malloc(_capacity * sizeof(PropertyItem)));
    memset(_properties, 0, _capacity * sizeof(PropertyItem));
    size_t numberArguments = sizeof...(Args);
    _buildProperties(numberArguments, args...);
  }

  PropertyCollection(const PropertyCollection&) = delete;

  ~PropertyCollection();

  // the number of properties stored, != capacity
  size_t numberProperties() const;

  // length of the serialized properties collection excluding length bytes
  size_t length() const;

  // get a pointer to the first PropertyItem. Might be nullptr or empty collection!
  const PropertyItem* get() const;

  // serialize to buffer, buffer should be large enough to hold the data (check with length())
  size_t serialize(uint8_t* buff) const;

  // parse supplied buff, return true on success
  bool deserialize(const uint8_t* buff, size_t len);

  // empty colelction and free linked memory (capacity stays unchanged)
  void reset();

 private:
  void _buildProperties(size_t num, ...);
  bool _propertyExists(uint8_t propertyId);
  void _addCapacity();

  PropertyItem* _properties;
  size_t _capacity;
  size_t _length;  // length needed to serialize excluding length bytes
};

}  // end namespace espMqttClientTypes
