extern crate xml;

use {Direction, Dock, Rect, Container, Split, SplitHandle, Workspace, DockHandle};
use self::xml::reader::{EventReader};
use self::xml::writer::{EventWriter, EmitterConfig, XmlEvent};
use self::xml::writer::Result as XmlResult;
use self::xml::reader::Result as XmlReaderResult;
use self::xml::reader::XmlEvent as XmlReaderEvent;
use self::xml::attribute::OwnedAttribute;
use std::fs::File;
use std::io::BufReader;
use std::io::prelude::*;

impl Rect {
    pub fn save<W: Write>(&self, writer: &mut EventWriter<W>) -> XmlResult<()> {
        try!(writer.write(XmlEvent::start_element("rect")
                          .attr("x", &format!("{}", self.x))
                          .attr("y", &format!("{}", self.y))
                          .attr("width", &format!("{}", self.width))
                          .attr("height", &format!("{}", self.height))));
        writer.write(XmlEvent::end_element())
    }

    // TODO(collin): Return error here

    pub fn load(attributes: &Vec<OwnedAttribute>) -> Rect {
        let mut rect = Rect::new(0.0, 0.0, 1024.0, 768.0);

        for attrib in attributes {
            match attrib.name.local_name.as_ref() {
                "x" => { rect.x = attrib.value.parse::<f32>().unwrap(); },
                "y" => { rect.y = attrib.value.parse::<f32>().unwrap(); },
                "width" => { rect.width = attrib.value.parse::<f32>().unwrap(); },
                "height" => { rect.height = attrib.value.parse::<f32>().unwrap(); },
                _ => (),
            }
        }

        rect
    }
}

impl Container {
    pub fn save<W: Write>(&self, name: &str, writer: &mut EventWriter<W>) -> XmlResult<()> {
        if self.docks.len() == 0 {
            return Ok(());
        }

        try!(writer.write(XmlEvent::start_element(name)));

        for dock in &self.docks {
            try!(writer.write(XmlEvent::start_element("dock")
                              .attr("handle", &format!("{}", dock.handle.0))
                              .attr("name", &format!("{}", dock.name))));
            try!(writer.write(XmlEvent::end_element()));
        }

        writer.write(XmlEvent::end_element())
    }

    pub fn load<E: Read>(parser: &mut EventReader<E>, side_name: &str) -> Container {
        let mut container = Container::new();

        //println!("Loding docks...");

        loop {
            match parser.next() {
                Ok(XmlReaderEvent::StartElement { name, attributes, .. }) => {
                    if name.local_name == "dock" {
                        let mut dock = Dock::new(DockHandle(0));

                        for attr in attributes {
                            match attr.name.local_name.as_ref() {
                                "handle" => dock.handle = DockHandle(attr.value.parse::<u64>().unwrap()),
                                "name" => dock.name = attr.value.to_owned(),
                                _ => (),
                            }
                        }

                        //println!("found dock!");

                        container.docks.push(dock);
                    }
                },
                Ok(XmlReaderEvent::EndElement { name }) => {
                    if name.local_name == side_name {
                        break;
                    }
                },
                Err(_) => break,

                _ => (),
            }
        }

        container
    }
}

impl Split {
    fn write_child<W: Write>(child: Option<SplitHandle>, writer: &mut EventWriter<W>, name: &str) -> XmlResult<()> {
        if child == None {
            try!(writer.write(XmlEvent::start_element(name).attr("con", "None")));
        } else {
            try!(writer.write(XmlEvent::start_element(name).attr("con", &format!("{}", child.unwrap().0))));
        }

        writer.write(XmlEvent::end_element())
    }

    pub fn save<W: Write>(&self, writer: &mut EventWriter<W>) -> XmlResult<()> {
        try!(writer.write(XmlEvent::start_element("split")));

        try!(writer.write(XmlEvent::start_element("ratio").attr("v", &format!("{}", self.ratio))));
        try!(writer.write(XmlEvent::end_element()));
        try!(writer.write(XmlEvent::start_element("direction").attr("v", &format!("{:?}", self.direction))));
        try!(writer.write(XmlEvent::end_element()));
        try!(writer.write(XmlEvent::start_element("handle").attr("v", &format!("{}", self.handle.0))));
        try!(writer.write(XmlEvent::end_element()));

        try!(self.left_docks.save("left_docks", writer));
        try!(self.right_docks.save("right_docks", writer));

        try!(Self::write_child(self.left, writer, "left"));
        try!(Self::write_child(self.right, writer, "right"));

        writer.write(XmlEvent::end_element())
    }

    pub fn parse_child(attributes: &Vec<OwnedAttribute>) -> Option<SplitHandle> {
        for attr in attributes {
            if attr.name.local_name == "con" {
                if attr.value == "None" {
                    return None;
                }

                return Some(SplitHandle(attr.value.parse::<u64>().unwrap()));
            }
        }

        None
    }

    pub fn load<E: Read>(reader: &mut EventReader<E>) -> Split {
        let mut split = Split::new(Direction::Full, 0.5, SplitHandle(0));

        //println!("loading split...");

        loop {
            match reader.next() {
                Ok(XmlReaderEvent::StartElement { name, attributes, .. }) => {
                    //println!("Name {}", name);
                    match name.local_name.as_ref() {
                        "left_docks" => split.left_docks = Container::load(reader, "left_docks"),
                        "right_docks" => split.right_docks = Container::load(reader, "right_docks"),
                        "left" => split.left = Self::parse_child(&attributes),
                        "right" => split.right = Self::parse_child(&attributes),
                        "ratio" => split.ratio = attributes[0].value.parse::<f32>().unwrap(),
                        "handle" => split.handle = SplitHandle(attributes[0].value.parse::<u64>().unwrap()),
                        "direction" => {
                            split.direction = match attributes[0].value.as_ref() {
                                "Vertical" => Direction::Vertical,
                                "Horizontal" => Direction::Horizontal,
                                _ => Direction::Full,
                            };
                        }
                        _ => (),
                    }
                },
                Ok(XmlReaderEvent::EndElement { name }) => {
                    if name.local_name == "split" {
                        break;
                    }
                },
                Err(_) => break,

                _ => (),
            }
        }

        split
    }
}

impl Workspace {
    pub fn save(&self, file_name: &str) -> XmlResult<()> {
        let mut file = try!(File::create(file_name));
        let mut w = EmitterConfig::new().perform_indent(true).create_writer(&mut file);

        try!(w.write(XmlEvent::start_element("workspace")));

        try!(self.rect.save(&mut w));

        //let mut t = String::new();
        //let t = format!("{}", self.rect.x);

        for split in &self.splits {
            try!(split.save(&mut w));
        }

        try!(w.write(XmlEvent::end_element()));

        println!("saved file");
        Ok(())
    }

    pub fn internal_load(&mut self, filename: &str) -> XmlReaderResult<()> {
        let file = File::open(filename).unwrap();
        let file = BufReader::new(file);

        let mut parser = EventReader::new(file);

        loop {
            match parser.next() {
                Ok(XmlReaderEvent::StartElement { name, attributes, .. }) => {
                    match name.local_name.as_ref() {
                        "rect" => self.rect = Rect::load(&attributes),
                        "split" => {
                            let split = Split::load(&mut parser);
                            //split.dump_info(0);
                            self.splits.push(split);
                        }
                        _ => (),
                    }
                },
                Ok(XmlReaderEvent::EndElement { name }) => {
                    if name.local_name == "workspace" {
                        break;
                    }
                },
                Err(e) => return Err(e),

                _ => (),
            }
        }

        println!("loaded ok!");

        Ok(())
    }

    pub fn load(filename: &str) -> Option<Workspace> {
        let mut ws = Self::new(Rect::new(0.0, 0.0, 1.0, 1.0)).unwrap();

        ws.splits.clear();

        if Self::internal_load(&mut ws, filename).is_ok() {
            Some(ws)
        } else {
            None
        }
    }
}

