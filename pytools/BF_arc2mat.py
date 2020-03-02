import re
import mat4py as sio
import sys

"""
Convert arcontrol_data.TXT to arcontrol_data.MAT. Just like "BF_arc2mat.m".
Xinfeng Chen, 2020-3-2

$ pip install mat4py
$ pyinstaller BF_arcmat.py
"""

def convert_txt2mat(filetxt):
    """
    Convert arcontrol_data.TXT to arcontrol_data.MAT. Just like "BF_arc2mat.m"
    :param filetxt:
    :return: None
    """
    pattern = re.compile(r'\.txt', flags=re.IGNORECASE)
    ind = pattern.findall(filetxt)
    assert len(ind)
    filemat = re.sub(pattern, '.mat', filetxt)

    # header #
    expression_header = re.compile('^@(IN\d+|OUT\d+|C\d+|C\d+S\d+):(.*)$')
    expression_taskname = re.compile('^-----(\w+)-----$')
    expression_arcbg  = re.compile(r'^ArC-bg$')
    MAT = {}
    MAT['info'] = {}
    isokfile = False
    for str in open(filetxt):
        res_header = re.findall(expression_header, str)
        res_taskname = re.findall(expression_taskname, str)
        res_arcbg = re.findall(expression_arcbg, str)
        if res_header:
            style, comment = res_header[0]
            MAT['info'][style] = comment
        elif res_taskname:
            MAT['info']['task'] = res_taskname[0]
        elif res_arcbg:
            isokfile = True
            break
    assert isokfile,  "It's NOT a data file from ArControl!"

    # data #
    expression = re.compile('^(IN\d+|OUT\d+|C\d+S\d+):(\w.*)$')
    for str in open(filetxt):
        res_expression = re.findall(expression, str)
        if res_expression:
            style, nums = res_expression[0]
            nums_list = eval('[' + nums.replace(' ', ', ') + ']')
            MAT.setdefault(style, []).append(nums_list)

    # save to file #
    sio.savemat(filemat, MAT)


if __name__ == '__main__':
    filetxts = sys.argv[1:]
    for filetxt in filetxts:
        convert_txt2mat(filetxt)
