def write_ini(filename, params):

    with open(filename, 'wb') as ini_file:
        ini_file.write(b"[directshow_metadata_injector_filter]\n")
        for k, v in params.items():
            ini_file.write("{}={}\n".format(k, v).encode())

