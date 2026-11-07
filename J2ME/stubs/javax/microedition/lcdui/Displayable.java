package javax.microedition.lcdui;
public abstract class Displayable {
    public void addCommand(Command c) {}
    public void setCommandListener(CommandListener l) {}
    public String getTitle() { return null; }
    public void setTitle(String t) {}
}
