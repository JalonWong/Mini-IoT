# coding=utf-8
import os
import sys


def LoadFile(file_name):
    try:
        file = open(file_name, 'rb')
        data_in = file.read()
    except FileNotFoundError as err:
        print("FileNotFoundError: {0}".format(err))
        return None
    return data_in


def SaveFile(file_name, data):
    print(data)
    file = open(file_name, 'w')
    file.write(data)
    file.close()


def ConvertFile(in_file_name, out_file_name):
    data = LoadFile(in_file_name)
    if data:
        data_out = ['static const u8 ', in_file_name.replace('.', '_'), '[] = {\n    ']
        i = 0
        for d in data:
            i += 1
            data_out.append('0x{:02x},'.format(d))
            if i % 20 == 0:
                data_out.append('\n    ')

        data_out.append('\n};\n')

        if not out_file_name:
            out_file_name = in_file_name.split('.')[0] + '.txt'
        SaveFile(out_file_name, ''.join(data_out))


def main():
    offset = 1
    param = sys.argv
    length = len(param)

    if length < 2:
        path = './'
        for filename in os.listdir(path):
            # print(filename+'====')
            sa = str.split(filename,'.')
            if len(sa) > 1 and (sa[1] == 'html'):
                # f = os.path.join(path,filename)
                # print("File path: " + f) #输出文件路径信息
                ConvertFile(filename, None)
        return

    if param[offset] == '-a':
        offset += 1
        print('All. TODO')
        return

    in_file_name = param[offset]
    offset += 1

    out_file_name = None
    if length > offset:
        out_file_name = param[offset]

    ConvertFile(in_file_name, out_file_name)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt as err:
        print("KeyboardInterrupt: {0}".format(err))

    os.system('pause')
