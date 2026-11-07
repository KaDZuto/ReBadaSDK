package javax.microedition.lcdui;
public class Display {
    public static Display getDisplay(Object midlet) { return new Display(); }
    public void setCurrent(Displayable d) {}
}
