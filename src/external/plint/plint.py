import xml.etree.ElementTree as ET
from jinja2 import FileSystemLoader,Environment
import argparse, webbrowser, os

class Message():
    prio = { "error": 1000, "warning": 100 }
    def __init__(self, type, code, desc, f, line):
        self.type = type
        self.code = code
        self.desc = desc
        self.file = f
        self.line = line

def sort_by_prio(msg):
    t = msg.type.lower()
    return -Message.prio[t] if t in Message.prio else 0

def transform(infile, outfile, template_file):

    env = Environment(loader = FileSystemLoader(["templates/"]), trim_blocks = True)

    # parse xml
    xmlfile = open(infile)
    if xmlfile is None:
        print("Could not open file " + infile)
        return

    parsed = ET.fromstring(xmlfile.read())
    messages = []
    for message in parsed.iter('message'):
        t = message.find('type').text
        code = message.find('code').text
        desc = message.find('desc').text
        f = message.find('file').text
        line = message.find('line').text

        m = Message(t, code, desc, f, line)
        messages.append(m)

    messages.sort(key = sort_by_prio)

    # render jinja template
    template = env.get_template(template_file)
    out = open(outfile, 'w', encoding='utf-8')
    out.write(template.render(messages = messages))
    out.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate html page from lint xml report.")
    parser.add_argument('infile', help='the input file to use')
    parser.add_argument('outfile', help='the output file to create')
    parser.add_argument('template', nargs='?', default="template", help='the name of the template to use')
    parser.add_argument('--show', action='store_true', help='open web page when done')
    arguments = parser.parse_args()

    print("transforming " + arguments.infile + " to " + arguments.outfile)
    transform(arguments.infile, arguments.outfile, arguments.template + ".html.jinja")

    if arguments.show:
        webbrowser.open_new_tab("file://" + os.path.abspath(arguments.outfile))

    print("done!")
