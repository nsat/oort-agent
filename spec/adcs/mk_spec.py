import yaml
import re
import sys

with open("adcs-fields.yml") as ad:
    fields = yaml.safe_load(ad)['adcs_hk']

oapi_types = {
    'acs_mode': 'type: string',
    'bool': 'type: boolean',
    'uint8_t': 'type: integer',
    'uint32_t': 'type: integer',
    'float': 'type: number',
    'float32': 'type: number',
    'double': 'type: number',
    'float64': 'type: number',
    'xyz_float_t': '"$ref": "adcs-schema.yml#/components/schemas/Adcs_xyz_float_t"',
    'xyz_int16_t': '"$ref": "adcs-schema.yml#/components/schemas/Adcs_xyz_int16_t"',
    'quat_t': '"$ref": "adcs-schema.yml#/components/schemas/Adcs_quat_t"',
    'euler_t': '"$ref": "adcs-schema.yml#/components/schemas/Adcs_euler_t"',
}
dsdl_types = {
    'bool': 'bool',
    'acs_mode': 'AcsMode',
    'uint8_t': 'uint8',
    'uint32_t': 'uint32',
    'float': 'float32',
    'float32': 'float32',
    'double': 'float64',
    'float64': 'float64',
    'xyz_float_t': 'XyzFloatT',
    'xyz_int16_t': 'XyzInt16T',
    'quat_t': 'QuatT',
}
cpp_types = {
    'bool': 'boolean',
    'uint8_t': 'uint8_t',
    'float32': 'float',
    'float64': 'double',
    'xyz_float_t': 'org::openapitools::server::model::Adcs_xyz_float_t',
    'euler_t': 'org::openapitools::server::model::Adcs_euler_t',
}

def indent(s, i):
    return i + s.replace('\n', '\n' + i)

if sys.argv[1] == 'dsdl':
    for r in fields:
        field = r
        ftype = fields[r]['type']
        if field and not fields[r].get('derived', False):
            print('{} {}'.format(dsdl_types[ftype], field))
            try:
                print(indent(fields[r]['desc'], '# '))
            except:
                print('### ')
            print('')
elif sys.argv[1] == 'oapi':
    idt = '  '
    si = '        '
    print("""
openapi: '3.0.3'
info: 
  title: ADCS data structures
  version: '1.0'

components:
  schemas:

    AdcsHk:
      description: ADCS hk response
      type: object
      properties:
""")
    for r in fields:
        field = r
        ftype = fields[r]['type']
        if field:
            print(si + '{}:'.format(field))
            print(si + idt + oapi_types[ftype])
            print(si + idt + 'description: >')
            try:
                print(indent(fields[r]['desc'], si + 2*idt))
            except:
                print(indent('adcs field', si + 2*idt))
            print('')
elif sys.argv[1] == 'spoc':
    print("""
def handler(e):
    adcs_hk = sat.lemadcs.adcs_hk()
    rsp = service.Response()
""")
    for r in fields:
        field = r
        if field:
            print("    rsp.{0} = adcs_hk['{0}']".format(field))
    print("    return rsp")
elif sys.argv[1] == 'cpp':
    # prototype derivations
    for r in fields:
        field = r
        if field:
            camelname = re.sub('(?:^|_)(.)', lambda m: m.group(1).upper(), field)
            if fields[r].get('derived', False):
                print("""
static {0} Derive{1}(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {{
    {0} value;
    // math here
    return value;
}}
""".format(cpp_types[fields[r]['type']], camelname))

    print("""
static void convert(
    org::openapitools::server::model::AdcsHk& oapi_hk,
    const ussp::payload::PayloadAdcsFeed& ucan_adcs) {
""")
    for r in fields:
        field = r
        if field:
            camelname = re.sub('(?:^|_)(.)', lambda m: m.group(1).upper(), field)
            if not fields[r].get('derived', False):
                print("    oapi_hk.set{0}(Adapt(ucan_adcs.{1}));".format(camelname, field))
    print("")
    for r in fields:
        field = r
        if field:
            camelname = re.sub('(?:^|_)(.)', lambda m: m.group(1).upper(), field)
            if fields[r].get('derived', False):
                print("    oapi_hk.set{0}(Derive{0}(oapi_hk, ucan_adcs));".format(camelname, field))

    print("}")
else:
    print("Unknown format")
