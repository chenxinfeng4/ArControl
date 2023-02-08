import re
import mat4py as sio
import sys

"""
Convert arcontrol_data.TXT to arcontrol_data.MAT. Just like "BF_arc2mat.m".
Xinfeng Chen, 2023-2-8

$ pip install mat4py
$ pyinstaller BF_arcmat.py
"""

def convert_txt2mat(filetxt):
    """
    Convert arcontrol_data.TXT to arcontrol_data.MAT. Just like "BF_arc2mat.m"
    :param filetxt:
    :return: None
    """
    # save to file #
    pattern = re.compile(r'\.txt', flags=re.IGNORECASE)
    ind = pattern.findall(filetxt)
    assert len(ind)
    filemat = re.sub(pattern, '.mat', filetxt)
    MAT = txt2MATdict(filetxt)
    sio.savemat(filemat, MAT)
    pass


def txt2MATdict(filetxt):
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
    expression_is_state = re.compile('^C\d+S\d+:(\w.*)$')
    expression_is_component = re.compile('^(C\d+S1|C0S0):(\w.*)$')
    previous_state = [None, 0, None]
    previous_component = [None, 0, None]
    endtime = 0

    for str in open(filetxt):
        res_expression = re.findall(expression, str)
        if res_expression:
            style, nums = res_expression[0]
            nums_list = eval('[' + nums.replace(' ', ', ') + ']')
            endtime = sum(nums_list)
            if re.findall(expression_is_component, str):
                cstyle = style.split('S')[0]
                pre_name = previous_component[0]
                if pre_name:
                    previous_component[2] = nums_list[0]-previous_component[1]
                    MAT.setdefault(previous_component[0], []).append(previous_component[1:])
                    previous_component = [cstyle, nums_list[0], 0]
                else:
                    previous_component = [cstyle, nums_list[0], 0]
            if re.findall(expression_is_state, str):
                pre_name = previous_state[0]
                if pre_name:
                    previous_state[2] = nums_list[0]-previous_state[1]
                    MAT.setdefault(previous_state[0], []).append(previous_state[1:])
                    previous_state = [style, nums_list[0], 0]
                else:
                    previous_state = [style, nums_list[0], 0]
            else:
                MAT.setdefault(style, []).append(nums_list)
        
    if previous_state[0]:
        previous_state[2] = endtime - previous_state[1]
        MAT.setdefault(previous_state[0], []).append(previous_state[1:])
        
    if previous_component[0]:
        previous_component[2] = endtime - previous_component[1]
        MAT.setdefault(previous_component[0], []).append(previous_component[1:])

    MAT['END'] = endtime
    return MAT


if __name__ == '__main__':
    filetxts = sys.argv[1:]
    for filetxt in filetxts:
        convert_txt2mat(filetxt)
