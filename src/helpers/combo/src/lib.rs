//! A wrapper to render a combo. Matches `variants` and `strings`, returning one of `variants` if
//! new item was selected in combo. Uses maximal string width as a combo width.

extern crate prodbg_api;
use prodbg_api::Ui;

pub fn combo<'a, T>(ui: &Ui,
                    id: &str,
                    variants: &'a [T],
                    strings: &[&str],
                    cur: &T)
                    -> Option<&'a T>
    where T: PartialEq
{

    if variants.len() != strings.len() {
        panic!("Variants and strings length should be equal in combo");
    }
    if variants.is_empty() {
        panic!("Variants cannot be empty in combo");
    }
    let mut res = None;
    let mut current_item = variants.iter().position(|var| var == cur).unwrap_or(0);
    let width = strings.iter().map(|s| ui.calc_text_size(s, 0).x as i32 + 40).max().unwrap_or(200);
    ui.push_item_width(width as f32);
    if ui.combo(id,
                &mut current_item,
                &strings,
                strings.len(),
                strings.len()) {
        res = Some(&variants[current_item]);
    }
    ui.pop_item_width();
    res
}
