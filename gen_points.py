#
#
#

def parse_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Generate Point Cloud")
    parser.add_argument('input', help='Input Images (Directory/Text)')
    parser.add_argument('output', help='Output Directory')
    return parser.parse_args()

def root_path():
    from os.path import dirname, abspath, realpath
    return dirname(abspath(__file__))

def run_vsfm(temp_root):
    """
    Structure from Motion
    """
    global ARGS
    from os.path import join
    import subprocess as sp
    vsfm_path = join(root_path(), 'vsfm/bin', 'VisualSFM')
    bundle_path = join(temp_root, 'bundle.nvm')
    print('Bundle Path: ' + bundle_path)
    sp.call([vsfm_path, 'sfm+shared+pmvs', ARGS.input, bundle_path])
    return bundle_path

def run_all():
    global ARGS
    ARGS = parse_args()
    #ARGS = {}
    #setattr(ARGS, input, join(root_path(), 'tmp/e100vs/images'))
    #setattr(ARGS, output, join(root_path(), 'tmp/'))

    from os.path import join, isdir, abspath
    from os import makedirs
    if not isdir(ARGS.output):
        makedirs(abspath(ARGS.output))

    from tempfile import mkdtemp
    #temp_root = mkdtemp()
    temp_root = join(abspath(ARGS.output), 'tmp')
    if not isdir(temp_root):
        makedirs(temp_root)
    print('Temporary Directory: ' + temp_root)

    try:
        #bundle_path = run_vsfm(temp_root)
        bundle_path = join(temp_root, 'bundle.nvm')

        from os.path import isfile
        if not isfile(bundle_path):
            raise "Failed to generate bundle"
        if not isdir(bundle_path + '.cmvs'):
            raise "Failed to process 1st pmvs"

        pmvs_data = join(bundle_path+'.cmvs', '00') + '/'
        pmvs_path = join(root_path(), 'vsfm/bin', 'pmvs2')
        genopt_path = join(root_path(), 'vsfm/bin', 'genOption')

        import shutil as sh
        import subprocess as sp

        sh.copy(join(pmvs_data, 'models', 'option-0000.ply'), join(ARGS.output, 'level-1.ply'))

        sp.call([genopt_path, pmvs_data, '0'])
        sp.call([pmvs_path, pmvs_data, 'option-0000'])
        sh.copy(join(pmvs_data, 'models', 'option-0000.ply'), join(ARGS.output, 'level-0.ply'))

        sp.call([genopt_path, pmvs_data, '2'])
        sp.call([pmvs_path, pmvs_data, 'option-0000'])
        sh.copy(join(pmvs_data, 'models', 'option-0000.ply'), join(ARGS.output, 'level-2.ply'))

        sp.call([genopt_path, pmvs_data, '3'])
        sp.call([pmvs_path, pmvs_data, 'option-0000'])
        sh.copy(join(pmvs_data, 'models', 'option-0000.ply'), join(ARGS.output, 'level-3.ply'))

    finally:
        from shutil import rmtree
        #rmtree(temp_root)
        print('Temporary Directory is removed')

if __name__ == '__main__':
    run_all()
