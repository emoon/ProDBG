
use PluginHandler;

pub struct Context {
    pub plugin_handler: PluginHandler
}

impl Context {
    pub fn new() -> Context {
        Context {
            plugin_handler: PluginHandler::new(), 
        }
    }
}






