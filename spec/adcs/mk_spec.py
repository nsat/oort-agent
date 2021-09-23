import csv
import sys

docs = list(csv.DictReader(open("adcs-v3.csv")))
mndoc = {d['mnumonic']:d for d in docs}

structure = list(csv.DictReader(open("lemadcs_adcs_hk.csv"), delimiter='\t'))

oapi_types = {
    'uint8_t': 'type: integer',
    'uint32_t': 'type: integer',
    'float': 'type: number',
    'double': 'type: number',
    'xyz_float_t': '"$ref": "adcs-schema.yml#/components/schemas/Adcs_xyz_float_t"',
    'xyz_int16_t': '"$ref": adcs-schema.yml"#/components/schemas/Adcs_xyz_int16_t"',
    'quat_t': '"$ref": adcs-schema.yml"#/components/schemas/Adcs_quat_t"',
}
dsdl_types = {
    'uint8_t': 'uint8',
    'uint32_t': 'uint32',
    'float': 'float32',
    'double': 'float64',
    'xyz_float_t': 'XyzFloatT',
    'xyz_int16_t': 'XyzInt16T',
    'quat_t': 'QuatT',
}

def indent(s, i):
    return i + s.replace('\n', '\n' + i)

if sys.argv[1] == 'dsdl':
    for r in structure:
        field = r['mnumonic']
        ftype = r['type']
        if field:
            print('{} {}'.format(dsdl_types[ftype], field))
            try:
                print(indent(mndoc[field]['description'], '# '))
                print(indent("unit = " + mndoc[field]['unit'], '#   '))
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
    for r in structure:
        field = r['mnumonic']
        ftype = r['type']
        if field:
            print(si + '{}:'.format(field))
            print(si + idt + oapi_types[ftype])
            print(si + idt + 'description: >')
            try:
                print(indent(mndoc[field]['description'], si + 2*idt))
                print(indent('unit = ' + mndoc[field]['unit'], si + 2*idt))
            except:
                print(indent('adcs field', si + 2*idt))
            print('')
elif sys.argv[1] == 'spoc':
    print("""
def handler(e):
    adcs_hk = sat.lemadcs.adcs_hk()
    rsp = service.Response()
""")
    for r in structure:
        field = r['mnumonic']
        if field:
            print("    rsp.{0} = adcs_hk['{0}']".format(field))
    print("    return rsp")
else:
    print("Unknown format")
