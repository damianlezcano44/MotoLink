#!/usr/bin/python

__author__ = 'Fabien Poussin'
__version__ = '0.2'


import re
from xml.etree import ElementTree as etree
import argparse
from jinja2 import Template
import pprint

pretty_print = pprint.PrettyPrinter(indent=2)


def pprint(*kwargs):
    pretty_print.pprint(kwargs)

PIN_MODE_INPUT = "PIN_MODE_INPUT({0})"
PIN_MODE_OUTPUT = "PIN_MODE_OUTPUT({0})"
PIN_MODE_ALTERNATE = "PIN_MODE_ALTERNATE({0})"
PIN_MODE_ANALOG = "PIN_MODE_ANALOG({0})"
PIN_ODR_LOW = "PIN_ODR_LOW({0})"
PIN_ODR_HIGH = "PIN_ODR_HIGH({0})"
PIN_OTYPE_PUSHPULL = "PIN_OTYPE_PUSHPULL({0})"
PIN_OTYPE_OPENDRAIN = "PIN_OTYPE_OPENDRAIN({0})"
PIN_OSPEED_VERYLOW = "PIN_OSPEED_VERYLOW({0})"
PIN_OSPEED_LOW = "PIN_OSPEED_LOW({0})"
PIN_OSPEED_MEDIUM = "PIN_OSPEED_MEDIUM({0})"
PIN_OSPEED_HIGH = "PIN_OSPEED_HIGH({0})"
PIN_PUPDR_FLOATING = "PIN_PUPDR_FLOATING({0})"
PIN_PUPDR_PULLUP = "PIN_PUPDR_PULLUP({0})"
PIN_PUPDR_PULLDOWN = "PIN_PUPDR_PULLDOWN({0})"
PIN_AFIO_AF = "PIN_AFIO_AF({0}, {1})"

FMT = '{0}'
FMT_DEF = '({0})'

PIN_CONF_LIST = ['MODER', 'OTYPER', 'OSPEEDR', 'PUPDR', 'ODR']
PIN_CONF_LIST_AF = ['AFRL', 'AFRH']

DEFAULT_PAD = {"SIGNAL": "UNUSED",
               "LABEL": "",
               "MODER": PIN_MODE_ANALOG,
               "OTYPER": PIN_OTYPE_PUSHPULL,
               "OSPEEDR": PIN_OSPEED_VERYLOW,
               "PUPDR": PIN_PUPDR_FLOATING,
               "ODR": PIN_ODR_HIGH}

PIN_MODE_TRANSLATE = {"GPIO_MODE_AF_PP": PIN_MODE_ALTERNATE,
                      "GPIO_MODE_ANALOG": PIN_MODE_ANALOG,
                      "GPIO_MODE_INPUT": PIN_MODE_INPUT,
                      "GPIO_MODE_OUTPUT": PIN_MODE_OUTPUT,
                      "GPIO_MODE_OUTPUT_PP": PIN_MODE_OUTPUT,
                      "GPIO_MODE_OUTPUT_OD": PIN_MODE_OUTPUT}

PIN_OTYPE_TRANSLATE = {"GPIO_MODE_OUTPUT_PP": PIN_OTYPE_PUSHPULL,
                        "GPIO_MODE_OUTPUT_OD": PIN_OTYPE_OPENDRAIN}

PIN_OSPEED_TRANSLATE = {"GPIO_SPEED_FREQ_LOW": PIN_OSPEED_VERYLOW,
                        "GPIO_SPEED_FREQ_MEDIUM": PIN_OSPEED_LOW,
                        "GPIO_SPEED_FREQ_HIGH": PIN_OSPEED_MEDIUM,
                        "GPIO_SPEED_FREQ_VERY_HIGH": PIN_OSPEED_HIGH
                        }

PIN_PUPDR_TRANSLATE = {"GPIO_NOPULL": PIN_PUPDR_FLOATING,
                       "GPIO_PULLUP": PIN_PUPDR_PULLUP,
                       "GPIO_PULLDOWN": PIN_PUPDR_PULLDOWN}

parser = argparse.ArgumentParser(description='Generate GPIO header file from STM32CubeMX file.')
parser.add_argument('-g', '--gpio', default='GPIO-STM32F303_gpio_v1_0_Modes.xml', type=str)
parser.add_argument('-b', '--project', default='../../board/board.ioc', type=str)
parser.add_argument('-o', '--output', default='board_gpio.h', type=str)


def open_xml(filename):
    #  Remove namespace
    with open(filename, 'r') as xmlfile:
        xml = re.sub(' xmlns="[^"]+"', '', xmlfile.read(), count=1)
    return etree.fromstring(xml)


def char_range(c1, c2):
    """Generates the characters from `c1` to `c2`, inclusive."""
    for c in range(ord(c1), ord(c2)+1):
        yield chr(c)


def read_gpio(filename):
    gpio = {'ports': {}, 'defaults': {}, 'modes': {}}
    root = open_xml(filename)

    gpio['defaults']['GPIO_Mode'] = 'GPIO_MODE_ANALOG'

    for modes in root.findall("RefParameter"):
        try:
            name = modes.attrib['Name']
            gpio['defaults'][name] = modes.attrib['DefaultValue']
            gpio['modes'][name] = []
        except KeyError as e:
            continue

        if 'GPIO_' not in name:
            continue

        for m in modes.findall("PossibleValue"):
            prop_val = m.attrib['Value']
            gpio['modes'][name].append(prop_val)

    for pin in root.findall('GPIO_Pin'):
        try:
            port = pin.attrib['Name'][1]
            num = int(pin.attrib['Name'][2:])
            if port not in gpio['ports']:
                gpio['ports'][port] = {}
            if num not in gpio['ports'][port]:
                gpio['ports'][port][num] = {}
        except ValueError as e:
            continue

        for s in pin.findall('PinSignal'):
            try:
                af = s.find('SpecificParameter/PossibleValue').text
                af = int(''.join(af.split('_')[1])[2:])
                gpio['ports'][port][num][s.attrib['Name']] = af
            except ValueError as e:
                print(e)
            except AttributeError as e:
                print(e)

    return gpio


# Extract signals from IOC
def read_project(gpio, filename):
    with open(filename, 'r') as mx_file:
        tmp = mx_file.readlines()
    pads = {}

    # Default all pads to analog
    for p in gpio['ports'].keys():
        pads[p] = {}
        for i in range(0, 16):
            pads[p][i] = DEFAULT_PAD.copy()
            pads[p][i]['PUPDR'] = PIN_PUPDR_TRANSLATE[gpio['defaults']['GPIO_PuPdOD']]
            pads[p][i]['OTYPER'] = PIN_OTYPE_TRANSLATE[gpio['defaults']['GPIO_ModeDefaultOutputPP']]
            pads[p][i]['OSPEEDR'] = PIN_OSPEED_TRANSLATE[gpio['defaults']['GPIO_Speed']]

    for t in tmp:
        if re.search(r"^P[A-Z]\d{1,2}(-OSC.+)?\.", t, re.M):
            split = t.split('=')
            pad_name = split[0].split(".")[0]
            pad_port = pad_name[1:2]
            pad_num = int(pad_name[2:4].replace('.', '').replace('-', ''))
            pad_prop = split[0].split(".")[-1]
            prop_value = split[-1].rstrip('\r\n')

            if pad_prop == "Signal":
                pads[pad_port][pad_num]["SIGNAL"] = prop_value
                pads[pad_port][pad_num]["MODER"] = PIN_MODE_ALTERNATE
                pads[pad_port][pad_num]["OSPEEDR"] = PIN_OSPEED_MEDIUM
            elif pad_prop == "GPIO_Mode":
                pads[pad_port][pad_num]["MODER"] = PIN_MODE_TRANSLATE[prop_value]
            elif pad_prop == "GPIO_Label":
                pads[pad_port][pad_num]["LABEL"] = prop_value
            elif pad_prop == "GPIO_PuPd":
                pads[pad_port][pad_num]["PUPDR"] = PIN_PUPDR_TRANSLATE[prop_value]
            elif pad_prop == "GPIO_ModeDefaultOutputPP":
                pads[pad_port][pad_num]["OTYPER"] = PIN_OTYPE_TRANSLATE[prop_value]
                pads[pad_port][pad_num]["MODER"] = PIN_MODE_OUTPUT
            elif pad_prop == "GPIO_Speed":
                pads[pad_port][pad_num]["OSPEEDR"] = PIN_OSPEED_TRANSLATE[prop_value]

    return pads


# Add defines for all pins with labels
def gen_defines(project):
    defines = {}

    for port_key in sorted(project.keys()):
        for pad_key in sorted(project[port_key].keys()):

            pad_data = project[port_key][pad_key]
            if pad_data['SIGNAL'] != 'UNUSED' and not pad_data['LABEL']:
                pad_data['LABEL'] = pad_data['SIGNAL']
            pad_data['LABEL'] = pad_data['LABEL'].replace('-', '_')
            label = pad_data['LABEL']
            signal = pad_data['SIGNAL']
            if not label:
                continue

            defines['PORT_'+label] = 'GPIO' + port_key
            defines['PAD_'+label] = pad_key

            if re.search(r"TIM\d_CH\d", signal, re.M):
                timer = signal.replace('S_TIM', '').replace('_CH', '')[:-1]
                timer_num = int(signal[-1:])

                defines['TIM_' + label] = 'TIM' + timer
                defines['CCR_' + label] = timer_num
                defines['PWMD_' + label] = 'PWMD' + timer
                defines['ICUD_' + label] = 'ICUD' + timer
                defines['CHN_' + label] = timer_num - 1
                print(label)

    return defines


# Each Port (A.B.C...)
def gen_ports(gpio, project):
    ports = {}
    for port_key in sorted(project.keys()):

        ports[port_key] = {}
        # Each property (mode, output/input...)
        for conf in PIN_CONF_LIST:
            ports[port_key][conf] = []
            for pin in project[port_key]:
                out = project[port_key][pin][conf]
                out = out.format(pin)
                ports[port_key][conf].append(out)

        conf = PIN_CONF_LIST_AF[0]
        ports[port_key][conf] = []
        for pin in range(0, 8):
            try:
                af = project[port_key][pin]['SIGNAL']
                out = PIN_AFIO_AF.format(pin, gpio['ports'][port_key][pin][af])
            except KeyError:
                out = PIN_AFIO_AF.format(pin, 0)
            ports[port_key][conf].append(out)

        conf = PIN_CONF_LIST_AF[1]
        ports[port_key][conf] = []
        for pin in range(8, 16):
            try:
                af = project[port_key][pin]['SIGNAL']
                out = PIN_AFIO_AF.format(pin, gpio['ports'][port_key][pin][af])
            except KeyError:
                out = PIN_AFIO_AF.format(pin, 0)
            ports[port_key][conf].append(out)

    return ports


if __name__ == '__main__':
    args = parser.parse_args()
    gpio = read_gpio(args.gpio)
    proj = read_project(gpio, args.project)
    defines = gen_defines(proj)
    ports = gen_ports(gpio, proj)

    with open('board_gpio.tpl', 'r') as tpl_file:
        tpl = tpl_file.read()
    template = Template(tpl)

    defines_sorted = []
    for d in sorted(defines.keys()):
        defines_sorted.append((d, defines[d]))

    template.stream(defines=defines_sorted, ports=ports.items()).dump(args.output)
