# pyinstaller export_task_to_graphviz_proxy.py --name export_task_to_graphviz --exclude-module export_task_to_graphviz --upx-exclude export_task_to_graphviz.py --add-data export_task_to_graphviz.py;src 
# This's a pyinstaller proxy of export_task_to_graphviz.py, so the src/export_task_to_graphviz.py can be upgrade.

import sys
import os

if __name__ == '__main__':
    path = os.path.abspath(os.path.dirname(sys.argv[0]))
    sys.path.append(os.path.join(path, 'src'))

from export_task_to_graphviz import main

if __name__ == '__main__':
    main(sys.argv[1])
