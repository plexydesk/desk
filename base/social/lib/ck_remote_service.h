/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@plexydesk.com>
*  Authored By  :
*
*  PlexyDesk is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  PlexyDesk is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with PlexyDesk. If not, see <http://www.gnu.org/licenses/lgpl.html>
*******************************************************************************/
#ifndef SERVICEINPUTDEFINITION_H
#define SERVICEINPUTDEFINITION_H

#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include "ck_url.h"

#include <social_kit_export.h>

namespace social_kit {

typedef std::map<cherry_kit::string, cherry_kit::string> query_parameter_map_t;
typedef std::vector<cherry_kit::string> string_list;

class DECL_SOCIAL_KIT_EXPORT service_query_parameters {
public:
  service_query_parameters() {}
  service_query_parameters(const service_query_parameters &copy) {
    m_parameter_map = copy.m_parameter_map;
  }

  virtual ~service_query_parameters() { m_parameter_map.clear(); }

  void insert(const cherry_kit::string &a_key, const cherry_kit::string &a_value) {
    m_parameter_map[a_key] = a_value;
  }

  std::vector<cherry_kit::string> keys() {
    std::vector<cherry_kit::string> rv;

    for (std::map<cherry_kit::string, cherry_kit::string>::iterator it =
             m_parameter_map.begin();
         it != m_parameter_map.end(); ++it) {
      rv.push_back(it->first);
    }

    return rv;
  }

  cherry_kit::string value(const cherry_kit::string &a_key) { return m_parameter_map[a_key]; }

  query_parameter_map_t data() const { return m_parameter_map; }

private:
  std::map<cherry_kit::string, cherry_kit::string> m_parameter_map;
};

//<attr ... >
class DECL_SOCIAL_KIT_EXPORT remote_data_attribute {
public:
  typedef enum {
    kBoolProperty,
    kCharProerpy,
    kStringProperty,
    kIntProperty,
    kRealProerpy
  } property_type_t;

  remote_data_attribute();
  remote_data_attribute(const remote_data_attribute &a_copy) {
    m_key = a_copy.m_key;
    m_value = a_copy.m_value;
    m_value_type = a_copy.m_value_type;
  }

  virtual ~remote_data_attribute();

  property_type_t type() const;
  void set_type(property_type_t a_type);

  cherry_kit::string key() const;
  void set_key(const cherry_kit::string &a_key);

  cherry_kit::string value() const;
  void set_value(const cherry_kit::string &a_value);

private:
  cherry_kit::string m_key;
  cherry_kit::string m_value;
  property_type_t m_value_type;
};

//<query>
typedef std::vector<remote_data_attribute> attribute_list_t;
class DECL_SOCIAL_KIT_EXPORT remote_result_data {
public:
  remote_result_data();
  remote_result_data(const remote_result_data &a_copy) {
    m_name = a_copy.m_name;
    m_propery_list = a_copy.m_propery_list;
  }

  virtual ~remote_result_data();

  cherry_kit::string name() const { return m_name; }

  void set_name(const cherry_kit::string &a_name) { m_name = a_name; }

  void insert(remote_data_attribute &a_attrib) {
    m_propery_list.push_back(a_attrib);
  }

  remote_data_attribute get(const cherry_kit::string &a_attribute_name);

private:
  std::vector<remote_data_attribute> m_propery_list;
  cherry_kit::string m_name;
};

//<result>
typedef std::vector<remote_result_data> result_list_t;
class DECL_SOCIAL_KIT_EXPORT remote_result {
public:
  remote_result();
  remote_result(const remote_result &a_copy);
  ~remote_result();

  void insert(const remote_result_data &a_data);

  std::vector<remote_result_data> query() const { return m_query_list; }

  result_list_t get(const cherry_kit::string &a_name) const;

private:
  std::vector<remote_result_data> m_query_list;
};

class DECL_SOCIAL_KIT_EXPORT remote_service {
public:
  typedef enum {
    kDefinitionLoadError,
    kNoError
  } definition_error_t;

  remote_service(const cherry_kit::string &input);
  virtual ~remote_service();

  string_list service_list() const;

  cherry_kit::string endpoint(const cherry_kit::string &a_name) const;

  url_request::url_request_type_t method(const cherry_kit::string &name) const;

  string_list arguments(const cherry_kit::string &name) const;

  string_list input_arguments(const cherry_kit::string &a_name,
                              bool a_optional = false);
  string_list optional_arguments(const cherry_kit::string &name) const;

  cherry_kit::string url(const cherry_kit::string &a_method,
                  service_query_parameters *a_params) const;

  remote_result response(const cherry_kit::string &a_method_name,
                         const url_response &a_response) const;

  definition_error_t error() const;

protected:
  void load_services();
  cherry_kit::string data_prefix() const;

private:
  class remote_service_context;
  remote_service_context *const ctx;
};
}

#endif // SERVICEINPUTDEFINITION_H
