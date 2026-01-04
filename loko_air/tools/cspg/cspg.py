import json

arg_in_json_file = "project.json"
arg_out_c_file = "settings.c"
arg_out_h_file = "settings.h"

#########################################################

GETTER_TEMPLATE = """
{0} settings_get_{1}(void) {{
    return _storage.settings.{1};
}}
"""
SETTER_TEMPLATE = """
void settings_set_{1}({0} {1}) {{

    _LOG(\"Set .{1} = %ld(0x%lX)\", {1}, {1});
    _storage.settings.{1} = {1};

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}}
"""
GETTER_PROTOTYPE_TEMPLATE = "{0} settings_get_{1}(void);\n"
SETTER_PROTOTYPE_TEMPLATE = "void settings_set_{1}({0} {1});\n"

#########################################################

POINTER_GETTER_TEMPLATE = """
void settings_get_{1}({0} {2}) {{
    _LOG_ARRAY(\"Get .{1} = ", &_storage.settings.{1}[0], sizeof(_storage.settings.{1}));
    memcpy((uint8_t*){1}, &_storage.settings.{1}[0], sizeof(_storage.settings.{1}));
}}
"""
POINTER_SETTER_TEMPLATE = """
void settings_set_{1}(const {0} {2}) {{

    _LOG_ARRAY(\"Set .{1} = ", {1}, sizeof(_storage.settings.{1}));
    memcpy(&_storage.settings.{1}, (uint8_t*){1}, sizeof(_storage.settings.{1}));

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}}
"""
POINTER_GETTER_PROTOTYPE_TEMPLATE = "void settings_get_{1}({0} {2});\n"
POINTER_SETTER_PROTOTYPE_TEMPLATE = "void settings_set_{1}(const {0} {2});\n"

#########################################################

BOOL_GETTER_TEMPLATE = """
{0} settings_is_{1}(void) {{
    return _storage.settings.{1};
}}
"""

BOOL_SETTER_TEMPLATE = """
void settings_set_{1}({0} is_{1}) {{

    _LOG(\"Set .{1} = %ld\", is_{1});
    _storage.settings.{1} = is_{1};

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}}
"""
BOOL_GETTER_PROTOTYPE_TEMPLATE = "{0} settings_is_{1}(void);\n"
BOOL_SETTER_PROTOTYPE_TEMPLATE = "void settings_set_{1}({0} is_{1});\n"

#########################################################

TYPEDEF_TEMPLATE = "    {0} {1};\n"
ENUM_TEMPLATE = "    SETTINGS_{0}_{1},\n"
ENUM_COUNT_TEMPLATE = "\n    SETTINGS_{0}_COUNT\n"
DEFAULT_VALUES_TEMPLATE = "    .{0} = {1},\n"
PRINT_INFO_TEMPLATE = "\n    _LOG(\"\\t.{0} = %ld\", _storage.settings.{0});"
PRINT_INFO_ARRAY_TEMPLATE = "\n    _LOG_ARRAY(\"\\t.{0}\", _storage.settings.{0}, {1});"
SEPARATOR = "\n/* -------------------------------------------------------------------------- */\n"

#########################################################

typedef = "typedef struct {\n"
default_list = "static const settings_t _default = {\n"
function_list = ""
function_prototype_list = ""
print_info_list = "_LOG(\"Settings packet size %d, packet size %d\", sizeof(_storage.settings), sizeof(_storage));\n"
substruct_definition = ""
defines = ""
enums = ""

#########################################################
def make_enum_typedef(enum_type_name, enum_list):
    enum_typedef = "typedef enum {\n"
    for enum in enum_list:
        enum_typedef = enum_typedef + ENUM_TEMPLATE.format(name.upper(), enum.upper())
    enum_typedef = enum_typedef + ENUM_COUNT_TEMPLATE.format(name.upper())
    enum_typedef = enum_typedef + "}} {0};\n\n".format(enum_type_name)
    return enum_typedef

def make_sub_struct_typedef(struct_name, substruct):
    substruct_typedef = "typedef struct {\n"
    for item in substruct:
        type = item["type"]
        name = item["name"]
        substruct_typedef = substruct_typedef + TYPEDEF_TEMPLATE.format(type, name)
    substruct_typedef = substruct_typedef + "}} __packed {0};\n".format(struct_name)
    return (substruct_typedef)

def make_sub_struct_initialization(substruct):
    substruct_default = "{\n"
    for item in substruct:
        type = item["type"]
        name = item["name"]
        default_value = item["default"]
        substruct_default = substruct_default + "    " + DEFAULT_VALUES_TEMPLATE.format(name, default_value)
    substruct_default = substruct_default + "    }"
    return substruct_default

with open(arg_in_json_file, "r") as read_file:
    jsettings = json.load(read_file)

arg_in_json_file = jsettings["in_json_file"]
arg_out_h_file = jsettings["out_h_file"]
arg_out_c_file = jsettings["out_c_file"]
print("C file: %s" % arg_out_c_file)
print("H file: %s" % arg_out_h_file)
print("JSON file: %s" % arg_in_json_file)

#print(jsettings)
for item in jsettings["settings"]:
    type = item["type"]
    name = item["name"]
    default_value = item["default"]
    #print("Item " + str(item))
    if "array_size" in item:
        define_name_size = "SETTINGS_{}_SIZE".format(name.upper())
        defines = defines + "#define {} ({})\n".format(define_name_size, item["array_size"])
        array_name =  name + "[{0}]".format(define_name_size)

        typedef = typedef + TYPEDEF_TEMPLATE.format(type, array_name)
        array_default_value = str("{" + "".join("0x{:02x}, ".format(x) for x in default_value) + "}").replace(", }", "}")
        default_list = default_list + DEFAULT_VALUES_TEMPLATE.format(name, array_default_value)
        function_list = function_list + POINTER_GETTER_TEMPLATE.format(type, name, array_name) + SEPARATOR
        function_list = function_list + POINTER_SETTER_TEMPLATE.format(type, name, array_name) + SEPARATOR
        function_prototype_list = function_prototype_list + POINTER_SETTER_PROTOTYPE_TEMPLATE.format(type, name, array_name)
        function_prototype_list = function_prototype_list + POINTER_GETTER_PROTOTYPE_TEMPLATE.format(type, name, array_name)
    elif type == "bool":
        typedef = typedef + TYPEDEF_TEMPLATE.format(type, name)
        default_list = default_list + DEFAULT_VALUES_TEMPLATE.format(name, default_value)
        function_list = function_list + BOOL_GETTER_TEMPLATE.format(type, name) + SEPARATOR
        function_list = function_list + BOOL_SETTER_TEMPLATE.format(type, name) + SEPARATOR
        function_prototype_list = function_prototype_list + BOOL_SETTER_PROTOTYPE_TEMPLATE.format(type, name)
        function_prototype_list = function_prototype_list + BOOL_GETTER_PROTOTYPE_TEMPLATE.format(type, name)
    elif type == "sub_struct":
        substruct_definition = substruct_definition + make_sub_struct_typedef(item["sub_struct_name"], item["type_definition"])
        init_val = make_sub_struct_initialization(item["type_definition"])
        default_list = default_list + DEFAULT_VALUES_TEMPLATE.format(name, init_val)

        struct_name = item["sub_struct_name"]
        typedef = typedef + TYPEDEF_TEMPLATE.format(struct_name, name)
        struct_name = struct_name + "*"

        function_list = function_list + POINTER_GETTER_TEMPLATE.format(struct_name, name, name) + SEPARATOR
        function_list = function_list + POINTER_SETTER_TEMPLATE.format(struct_name, name, name) + SEPARATOR
        function_prototype_list = function_prototype_list + POINTER_SETTER_PROTOTYPE_TEMPLATE.format(struct_name, name, name)
        function_prototype_list = function_prototype_list + POINTER_GETTER_PROTOTYPE_TEMPLATE.format(struct_name, name, name)
    elif type == "enum":
        enum_type_name = "settings_" + name + "_t"
        enums = enums + make_enum_typedef(enum_type_name, item["enum"])

        typedef = typedef + TYPEDEF_TEMPLATE.format(enum_type_name, name)
        default_list = default_list + DEFAULT_VALUES_TEMPLATE.format(name, default_value)
        function_list = function_list + GETTER_TEMPLATE.format(enum_type_name, name) + SEPARATOR
        function_list = function_list + SETTER_TEMPLATE.format(enum_type_name, name) + SEPARATOR
        function_prototype_list = function_prototype_list + SETTER_PROTOTYPE_TEMPLATE.format(enum_type_name, name)
        function_prototype_list = function_prototype_list + GETTER_PROTOTYPE_TEMPLATE.format(enum_type_name, name)
    else:
        typedef = typedef + TYPEDEF_TEMPLATE.format(type, name)
        default_list = default_list + DEFAULT_VALUES_TEMPLATE.format(name, default_value)
        function_list = function_list + GETTER_TEMPLATE.format(type, name) + SEPARATOR
        function_list = function_list + SETTER_TEMPLATE.format(type, name) + SEPARATOR
        function_prototype_list = function_prototype_list + SETTER_PROTOTYPE_TEMPLATE.format(type, name)
        function_prototype_list = function_prototype_list + GETTER_PROTOTYPE_TEMPLATE.format(type, name)

    if "array_size" in item:
        print_info_list = print_info_list + "\n    //TODO array:" + name #PRINT_INFO_ARRAY_TEMPLATE.format(name, item["array_size"])
    else:
        print_info_list = print_info_list + PRINT_INFO_TEMPLATE.format(name)
    function_prototype_list = function_prototype_list + "\n"

default_list = default_list + "};"
typedef = typedef + "} __packed settings_t;\n"

#print("\r\n\r\n" + typedef)
# print("\r\n\r\n" + default_list)
# print("\r\n\r\n" + function_list)
# print("\r\n\r\n" + function_prototype_list)
# print("\r\n\r\n" + print_info_list)

#########################################################

c_file = ""
with open("settings.template.c", "r") as file:
    c_file = file.read()
c_file = c_file.replace( "/*__SETTINGS_C_DEFAUL_SETTINGS__*/", default_list)
c_file = c_file.replace( "/*__SETTINGS_C_LOG_INFO_PRINTS__*/", print_info_list)
c_file = c_file.replace( "/*__SETTINGS_C_GETTER_SETTER_FUNCTIONS__*/", function_list)
with open(arg_out_c_file, "w") as file:
     file.write(c_file)

#########################################################

h_file = ""
with open("settings.template.h", "r") as file:
    h_file = file.read()
h_file = h_file.replace( "/*__SETTINGS_H_HEADERS__*/", defines)
h_file = h_file.replace( "/*__SETTINGS_H_ENUMS__*/", enums)
h_file = h_file.replace( "/*__SETTINGS_H_STRUCTS__*/", substruct_definition)
h_file = h_file.replace( "/*__SETTINGS_H_SETTINGS_STRUCT__*/", typedef)
h_file = h_file.replace( "/*__SETTINGS_H_PROTOTUPES__*/", function_prototype_list)
with open(arg_out_h_file, "w") as file:
    file.write(h_file)
