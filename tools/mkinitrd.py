import os, struct, sys

def create_initrd(src_dir, output_file):


    files = []
    
    for f in os.listdir(src_dir):
        if os.path.isfile(os.path.join(src_dir, f)):
            files.append(f)


    with open(output_file, 'wb') as out:
        out.write(struct.pack('<I', len(files)))

        for f in files:
            path = os.path.join(src_dir, f)
            size = os.path.getsize(path)

            name_bytes = f.encode('utf-8')[:63].ljust(64, b'\0')
            out.write(name_bytes + struct.pack('<I', size))


        for f in files:
            with open(os.path.join(src_dir, f), 'rb') as in_f:
                out.write(in_f.read())

if __name__ == '__main__':
    create_initrd(sys.argv[1], sys.argv[2])
