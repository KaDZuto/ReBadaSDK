import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;

public class HelloMIDlet extends MIDlet implements CommandListener {
    private Display display;
    private Form form;
    private Command exit;

    protected void startApp() {
        display = Display.getDisplay(this);
        form = new Form("Hello J2ME");
        form.append(new StringItem(null, "Hello From 2026 (J2ME)"));
        exit = new Command("Exit", Command.EXIT, 1);
        form.addCommand(exit);
        form.setCommandListener(this);
        display.setCurrent(form);
    }

    protected void pauseApp() {}

    protected void destroyApp(boolean unconditional) {}

    public void commandAction(Command c, Displayable d) {
        if (c == exit) {
            destroyApp(true);
            notifyDestroyed();
        }
    }
}
