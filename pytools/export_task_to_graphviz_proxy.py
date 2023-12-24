# pyinstaller export_task_to_graphviz_proxy.py --name export_task_to_graphviz --exclude-module export_task_to_graphviz --upx-exclude export_task_to_graphviz.py --add-data export_task_to_graphviz.py;src 
# This's a pyinstaller proxy of export_task_to_graphviz.py, so the src/export_task_to_graphviz.py can be upgrade.

import graphviz
import os
from xml.dom import minidom
from collections import defaultdict
import sys

if __name__ == '__main__':
    path = os.path.abspath(os.path.dirname(sys.argv[0]))
    sys.path.append(os.path.join(path, 'src'))

    executable_path = os.path.join(path, 'graphviz_bin')
    existing_path = os.environ.get('PATH', '')
    path_separator = ';' if os.name == 'nt' else ':' # 根据操作系统设定分隔符
    os.environ['PATH'] = f"{executable_path}{path_separator}{existing_path}"

from export_task_to_graphviz import main

if __name__ == '__main__':
    main(sys.argv[1])
