/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Property.h"

namespace espMqttClientTypes {

enum PropertyType {
  BYTE,
  TWOBYTE,
  FOURBYTE,
  VARBYTE,
  STRING,
  STRINGPAIR,
  BINARY,
  N_A
};

constexpr struct PropertyIdtoType {
  uint8_t id;
  PropertyType type;
} propertyIdtoType[] = {
  PropertyId.PayloadFormatIndicator,          BYTE,
  PropertyId.MessageExpiryInterval,           FOURBYTE,
  PropertyId.ContentType,                     STRING,
  PropertyId.ResponseTopic,                   STRING,
  PropertyId.CorrelationData,                 BINARY,
  PropertyId.SubscriptionIdentifier,          VARBYTE,
  PropertyId.SessionExpiryInterval,           FOURBYTE,
  PropertyId.AssignedClientId,                STRING,
  PropertyId.ServerKeepAlive,                 TWOBYTE,
  PropertyId.AuthenticationMethod,            STRING,
  PropertyId.AuthenticationData,              BINARY,
  PropertyId.RequestProblemInformation,       BYTE,
  PropertyId.WillDelay,                       FOURBYTE,
  PropertyId.RequestResponse,                 BYTE,
  PropertyId.ResponseInformation,             STRING,
  PropertyId.ServerReference,                 STRING,
  PropertyId.ReasonString,                    STRING,
  PropertyId.ReceiveMaximum,                  TWOBYTE,
  PropertyId.TopicAliasMaximum,               TWOBYTE,
  PropertyId.TopicAlias,                      TWOBYTE,
  PropertyId.MaximumQoS,                      BYTE,
  PropertyId.RetainAvailable,                 BYTE,
  PropertyId.UserProperty,                    STRINGPAIR,
  PropertyId.MaximumPacketSize,               FOURBYTE,
  PropertyId.WildcardSubcriptionAvailable,    BYTE,
  PropertyId.SubscriptionIdentifierAvailable, BYTE,
  PropertyId.SharedSubscriptionAvailable,     BYTE
};

inline PropertyType getTypefromPropertyId(uint8_t id) {
  for (size_t i = 0; i < (sizeof(propertyIdtoType) / sizeof(propertyIdtoType[0])); ++i) {
    if (id == propertyIdtoType[i].id) {
      return propertyIdtoType[i].type;
    }
  }
  return N_A;
}

// duplicates c-string (incl terminator)
inline char* dupString(const char* str) {
  if (!str) return nullptr;
  size_t len = strlen(str) + 1;
  char* tmp = reinterpret_cast<char*>(malloc(len));
  if (tmp) {
    memcpy(tmp, str, len);
    return tmp;
  }
  return nullptr;
}

inline char* dupString(const uint8_t* data, size_t len) {
  if (!data) return nullptr;
  char* tmp = reinterpret_cast<char*>(malloc(len + 1));
  tmp[len] = '\0';
  if (tmp) {
    memcpy(tmp, data, len);
    return tmp;
  }
  return nullptr;
}

// duplicates byte array
inline uint8_t* dupData(const uint8_t* data, size_t len) {
  if (!data) return nullptr;
  uint8_t* tmp = reinterpret_cast<uint8_t*>(malloc(len));
  if (tmp) {
    memcpy(tmp, data, len);
    return tmp;
  }
  return nullptr;
}

PropertyCollection::PropertyCollection()
: _properties(nullptr)
, _capacity(NUMBER_PROPERTIES)
, _length(0) {
  _properties = reinterpret_cast<PropertyItem*>(malloc(_capacity * sizeof(PropertyItem)));
  memset(_properties, 0, _capacity * sizeof(PropertyItem));
  if (!_properties) {
    abort();
  }
}

PropertyCollection::PropertyCollection(size_t capacity)
: _properties(nullptr)
, _capacity(capacity)
, _length(0) {
  _properties = reinterpret_cast<PropertyItem*>(malloc(_capacity * sizeof(PropertyItem)));
  memset(_properties, 0, _capacity * sizeof(PropertyItem));
  if (!_properties) {
    abort();
  }
}

PropertyCollection::~PropertyCollection() {
  if (!_properties) return;
  reset();
  free(_properties);
}

size_t PropertyCollection::numberProperties() const {
  size_t num = 0;
  while (num < _capacity && _properties[num].propertyId > 0x00) {
    ++num;
  }
  return num;
}

size_t PropertyCollection::length() const {
  return _length;
}

const PropertyItem* PropertyCollection::get() const {
  return _properties;
}

size_t PropertyCollection::serialize(uint8_t* buff) const {
  if (!buff || !_properties) return 0;

  size_t index = 0;
  size_t i = 0;
  while (i < _capacity && _properties[i].propertyId > 0x00) {
    uint8_t id = buff[index++] = _properties[i].propertyId;
    PropertyType type = getTypefromPropertyId(id);
    switch(type) {
      case BINARY:
        buff[index++] = _properties[i].data.binaryData.length >> 8;
        buff[index++] = _properties[i].data.binaryData.length & 0xFF;
        memcpy(&buff[index], _properties[i].data.binaryData.data, _properties[i].data.binaryData.length);
        index += _properties[i].data.binaryData.length;
        break;
      case STRING:
        index += espMqttClientInternals::encodeString(_properties[i].data.utf8String, &buff[index]);
        break;
      case STRINGPAIR:
        index += espMqttClientInternals::encodeString(_properties[i].data.utf8StringPair.dataKey, &buff[index]);
        index += espMqttClientInternals::encodeString(_properties[i].data.utf8StringPair.dataValue, &buff[index]);
        break;
      case BYTE:
      case VARBYTE:  // varByte integer but in MQTT 5.0 spec this is always only 1 byte
        buff[index++] = _properties[i].data.byteCode;
        break;
      case TWOBYTE:
        buff[index++] = _properties[i].data.twoByteCode >> 8;
        buff[index++] = _properties[i].data.twoByteCode & 0xFF;
        break;
      case FOURBYTE:
        buff[index++] = _properties[i].data.fourByteCode >> 24;
        buff[index++] = _properties[i].data.fourByteCode >> 16;
        buff[index++] = _properties[i].data.fourByteCode >> 8;
        buff[index++] = _properties[i].data.fourByteCode & 0xFF;
        break;
      default:
        break;
    }
    ++i;
  }
  return index;
}

bool PropertyCollection::deserialize(const uint8_t* buff, size_t len) {
  size_t i = 0;
  size_t numProperties = 0;
  while (i < len) {
    if (numProperties == _capacity) {
      _addCapacity();
    }
    uint8_t id = buff[i++];
    if (id != PropertyId.UserProperty && _propertyExists(id)) {
      reset();
      return false;
    }
    PropertyType type = getTypefromPropertyId(id);
    if (type == N_A) {
      reset();
      return false;
    }
    _properties[numProperties].propertyId = id;
    if (type == BINARY) {
      _properties[numProperties].data.binaryData.length = buff[i++] << 8;
      _properties[numProperties].data.binaryData.length |= buff[i++];
      _properties[numProperties].data.binaryData.data = dupData(&buff[i], _properties[numProperties].data.binaryData.length);
      i += _properties[numProperties].data.binaryData.length;
    } else if (type == STRING) {
      size_t length = buff[i++] << 8;
      length |= buff[i++];
      _properties[numProperties].data.utf8String = dupString(&buff[i], length);
      i += length;
    } else if (type == STRINGPAIR) {
      size_t length = buff[i++] << 8;
      length |= buff[i++];
      _properties[numProperties].data.utf8StringPair.dataKey = dupString(&buff[i], length);
      i += length;
      length = buff[i++] << 8;
      length |= buff[i++];
      _properties[numProperties].data.utf8StringPair.dataValue = dupString(&buff[i], length);
      i += length;
    } else if (type == BYTE || type == VARBYTE) {
      _properties[numProperties].data.byteCode = buff[i++];
    } else if (type == TWOBYTE) {
      _properties[numProperties].data.twoByteCode = buff[i++] << 8;
      _properties[numProperties].data.twoByteCode |= buff[i++];
    } else if (type == FOURBYTE) {
      _properties[numProperties].data.fourByteCode = buff[i++] << 24;
      _properties[numProperties].data.fourByteCode |= buff[i++] << 16;
      _properties[numProperties].data.fourByteCode |= buff[i++] << 8;
      _properties[numProperties].data.fourByteCode |= buff[i++];
    }
    numProperties++;
    if (i > len) return false;
  }
  return true;
}

void PropertyCollection::reset() {
  size_t i = 0;
  while (i < _capacity && _properties[i].propertyId > 0x00) {
    PropertyType type = getTypefromPropertyId(_properties[i].propertyId);
    if (type == BINARY) {
      free(_properties[i].data.binaryData.data);
    } else if (type == STRING) {
      free(_properties[i].data.utf8String);
    } else if (type == STRINGPAIR) {
      free(_properties[i].data.utf8StringPair.dataKey);
      free(_properties[i].data.utf8StringPair.dataValue);
    }
    ++i;
  }
  memset(_properties, 0, _capacity * sizeof(PropertyItem));
}

void PropertyCollection::_buildProperties(size_t num, ...) {
  va_list argList;
  va_start(argList, num);
  int32_t i = num;
  size_t numberProperties = 0;
  while (i > 0) {
    if (numberProperties == _capacity) {
      _addCapacity();
    }
    uint8_t id = va_arg(argList, int) & 0xFF;  // property id, is uint8_t but promoted to int
    PropertyType type = getTypefromPropertyId(id);
    _properties[numberProperties].propertyId = id;
    ++_length;
    if (type == BINARY) {
      uint8_t* data = va_arg(argList, uint8_t*);  // property data
      size_t dataLen = va_arg(argList, size_t);  // size data
      _properties[numberProperties].data.binaryData.length = dataLen;
      _properties[numberProperties++].data.binaryData.data = dupData(data, dataLen);
      _length += 2 + dataLen;
      i -= 3;
    } else if (type == STRING) {
      const char* data = va_arg(argList, const char*);  // property data
      _properties[numberProperties++].data.utf8String = dupString(data);
      _length += 2 + strlen(data);
      i -= 2;
    } else if (type == STRINGPAIR) {
      const char* data = va_arg(argList, const char*);  // property key
      _properties[numberProperties].data.utf8StringPair.dataKey = dupString(data);
      _length += 2 + strlen(data);
      data = va_arg(argList, const char*);  // property data
      _properties[numberProperties++].data.utf8StringPair.dataValue = dupString(data);
      _length += 2 + strlen(data);
      i -= 3;
    } else if (type == BYTE || type == VARBYTE) {
      int data = va_arg(argList, int);  // property value
      i -= 2;
      _properties[numberProperties++].data.byteCode = data & 0xFF;
      _length += 1;
    } else if (type == TWOBYTE) {
      int data = va_arg(argList, int);  // property value
      i -= 2;
      _properties[numberProperties++].data.twoByteCode = static_cast<uint16_t>(data & 0xFFFF);
      _length += 2;
    } else if (type == FOURBYTE) {
      int data = va_arg(argList, int);  // property value
      i -= 2;
      _properties[numberProperties++].data.fourByteCode = static_cast<uint32_t>(data & 0xFFFFFFFF);
      _length += 4;
    }
  }
  va_end(argList);
}

bool PropertyCollection::_propertyExists(uint8_t propertyId) {
  size_t i = 0;
  while (i < _capacity && _properties[i].propertyId > 0x00) {
    if (propertyId == _properties[i++].propertyId) {
      return true;
    }
  }
  return false;
}

void PropertyCollection::_addCapacity() {
  PropertyItem* newProperties = reinterpret_cast<PropertyItem*>(realloc(_properties, (_capacity + 5) * sizeof(PropertyItem)));
  if (!newProperties) {
    abort();
  }
  memset(&_properties[_capacity], 0, 5 * sizeof(PropertyItem));
}

}  // end namespace espMqttClientTypes
