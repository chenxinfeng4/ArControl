import graphviz
import os
from xml.dom import minidom
from collections import defaultdict
import sys

filepath = 'go_gono.aconf'



def getDesinate(FromCi, strip):
    nextstate = strip.getElementsByTagName('nextstate')[0]
    staterandtype = nextstate.getAttribute('type')
    iscomponent = nextstate.getAttribute('iscomponent').lower() == 'true'
    nextstateprefix = 'C{}S1' if iscomponent else  f'C{FromCi}S'+'{}'
    if staterandtype=='fixed':
        number = nextstate.getElementsByTagName('fixed')[0].getAttribute('fixed')
        number = int(number)
        nextstate_numbers = [nextstateprefix.format(number)]
    elif staterandtype=='range':
        number_from = nextstate.getElementsByTagName('range')[0].getAttribute('from')
        number_to = nextstate.getElementsByTagName('range')[0].getAttribute('to')
        number_from = int(number_from)
        number_to = int(number_to)
        numbers = list(range(number_from, number_to+1))
        nextstate_numbers = [nextstateprefix.format(number) for number in numbers]
    elif staterandtype=='goelse':
        number_go = nextstate.getElementsByTagName('goelse')[0].getAttribute('go')
        number_else = nextstate.getElementsByTagName('goelse')[0].getAttribute('else')
        number_go = int(number_go)
        number_else = int(number_else)
        numbers = [number_go, number_else]
        nextstate_numbers = [nextstateprefix.format(number) for number in numbers]
    elif staterandtype=='endup':
        nextstate_numbers = ['final']
    else:
        raise 'Not vernified nextstate type.'
    return nextstate_numbers

def getRandDestinateNumber(strip) -> str:
    domtype = strip.getAttribute('type')
    if domtype=='fixed':
        number = strip.getElementsByTagName('fixed')[0].getAttribute('fixed')
        res = number
    elif domtype=='range':
        number_from = strip.getElementsByTagName('range')[0].getAttribute('from')
        number_to = strip.getElementsByTagName('range')[0].getAttribute('to')
        res = f'{number_from}-{number_to}'
    elif domtype=='goelse':
        number_go = strip.getElementsByTagName('goelse')[0].getAttribute('go')
        number_else = strip.getElementsByTagName('goelse')[0].getAttribute('else')
        res = f'{number_go} OR {number_else}'
    else:
        raise 'Not vernified nextstate type.'
    return res


def next_state_trans(fromci, fromsi, stcomment, nextStateNames) -> list:
    if len(nextStateNames)>1:
        stcomment = stcomment + ' (random)'
    trans_l = [(f'C{fromci}S{fromsi}', nextcisi, ' '+stcomment) for nextcisi in nextStateNames]
    return trans_l


def convert(filepath):
    dot = graphviz.Digraph(comment='The Round Table', filename=filepath.replace('aconf', 'gv'))
    dot.attr('node', shape='Mrecord')
    
    file = minidom.parse(filepath)

    root = file.getElementsByTagName('ARC_DESIGNER')[0]
    session = root.getElementsByTagName('SESSION')[0]
    components = session.getElementsByTagName('COMPONENT')

    components_d = {}
    auto_hide_C1 = 'cluster_C' if len(components)>=2 else 'C'
    for ci, component in enumerate(components, start=1):
        name = component.getAttribute('comment')
        states = component.getElementsByTagName('STATE')
        states_d = {}
        components_d[ci] = (name, states_d)
        component_label = f'C{ci} : {name}' if name else f'C{ci}'
        with dot.subgraph(name=f'{auto_hide_C1}{ci}') as subg:
            subg.attr(style='rounded', color='lightgrey', label=component_label)

            for si, state in enumerate(states, start=1):
                sname = state.getAttribute('comment')
                sid = f'C{ci}S{si}'
                state_label = f'S{si} : {sname}' if sname else f'S{si}'

                strips = state.getElementsByTagName('STATE_STRIP')
                strips_d = defaultdict(list)
                for sti, strip in enumerate(strips):
                    isEnable = strip.getAttribute('isEnable').lower() == 'true'
                    strtype =  strip.getAttribute('type')
                    if not isEnable:
                        continue
                    stname = strip.getAttribute('comment')
                    
                    if stname:
                        strips_d[strtype].append(stname)
                        continue
                    if strtype=='doVar':
                        strips_d[strtype].append('doVar')
                        continue
                    if strtype=='doPin':
                        item = strip.getElementsByTagName('doPin')[0]
                        pinnum = item.getAttribute('number')
                        pinmode = item.getAttribute('type')
                        stname = f'OUT{pinnum} {pinmode}'
                        strips_d[strtype].append(stname)
                
                comments_l = []
                for stname in strips_d['doVar']:
                    comments_l.append(f'+ {stname} \l')
                    
                for stname in strips_d['doPin']:
                    comments_l.append(f'+ {stname} \l')
                comments_str = "".join(comments_l)
                comments_str = comments_str.replace('"','').replace("'","")
                if comments_str:
                    state_label = '{'+f'{state_label}|{comments_str}'+'}'
                
                subg.node(sid, label=state_label)

    #%% 定义开始和结束
    dot.attr('node', shape='ellipse', color='black', style='filled')
    dot.node('initial', label='', height="0.2", width="0.2")
    dot.attr('node', shape='doublecircle')
    dot.node('final', label='', fixedsize="shape", height="0.2", width="0.2")

    #%% 定义转变
    for ci, component in enumerate(components, start=1):
        name = component.getAttribute('comment')
        states = component.getElementsByTagName('STATE')
        for si, state in enumerate(states, start=1):
            CiSi_id = f'C{ci}S{si}'
            strips = state.getElementsByTagName('STATE_STRIP')
            for sti, strip in enumerate(strips):
                isEnable = strip.getAttribute('isEnable').lower() == 'true'
                strtype =  strip.getAttribute('type')
                if not isEnable:
                    continue

                if not strtype.startswith('when'):
                    continue

                nextStateNames = getDesinate(ci, strip)
                stcomment = strip.getAttribute('comment').strip()
                stcomment = stcomment + '->'
                ind_arrow = stcomment.find('->')
                if ind_arrow>0:  #has comment already
                    stcomment = stcomment[:ind_arrow]
                    stcomment = stcomment.strip()
                    trans_l_now = next_state_trans(ci, si, stcomment, nextStateNames)
                    for trans_now in trans_l_now:
                        dot.edge(*trans_now)
                    continue

                if strtype=='whenPin':
                    number = strip.getElementsByTagName(strtype)[0].getAttribute('number')
                    stcomment = f'IN{number}'
                elif strtype=='whenVar':
                    stcomment = 'whenVar'
                elif strtype=='whenTime':
                    substrip = strip.getElementsByTagName('time')[0]
                    rightcomment = getRandDestinateNumber(substrip)
                    stcomment = f't = {rightcomment}'
                elif strtype=='whenCount':
                    substrip = strip.getElementsByTagName('count')[0]
                    rightcomment = getRandDestinateNumber(substrip)
                    if rightcomment=='1':
                        stcomment = ''
                    else:
                        stcomment = f'N = {rightcomment}'
                trans_l_now = next_state_trans(ci, si, stcomment, nextStateNames)
                for trans_now in trans_l_now:
                    dot.edge(*trans_now)
    dot.edge('initial', 'C1S1')
    dot.view()


def main(path):
    if os.path.isfile(path):
        xml_file = path
    elif os.path.isdir(path):
        xml_file = os.path.join(path, os.path.basename(path) + '.aconf')
        if not os.path.isfile(xml_file):
            print('Error: no xml file in this directory')
            sys.exit(1)
    else:
        print('Error: path is not a file or directory')
        sys.exit(1)
    
    convert(xml_file)


if __name__ == '__main__':
    path = sys.argv[1]
    main(path)
