#include <stdio.h>
#include "../src/util.c"

int main()
{
    char *yml_filename = "test/basic-record.yml";
    convert_file_yaml_to_cyaml(yml_filename, ...);
    return 0;
}
