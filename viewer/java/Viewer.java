/*
 * (c)2018, Wouter Caarls
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.diplib.viewer;

import com.sun.jna.Pointer;
import com.sun.jna.Native;
import com.sun.jna.Library;
import com.sun.jna.Callback;
import com.sun.jna.NativeLibrary;

import java.io.File;

import com.jogamp.opengl.GL2;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLCapabilities;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.GLProfile;
import com.jogamp.opengl.awt.GLCanvas;

import java.awt.Point;

import java.awt.event.WindowEvent;
import java.awt.event.InputEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.event.KeyEvent;
import java.awt.event.WindowListener;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelListener;
import java.awt.event.KeyListener;

import javax.swing.JFrame;

/// JNA interface to ProxyManager
interface Proxy extends Library {

    interface SwapBuffersCallback extends Callback {
        void invoke();
    }

    interface SetWindowTitleCallback extends Callback {
        void invoke(String title);
    }

    interface RefreshWindowCallback extends Callback {
        void invoke();
    }

    interface CreateWindowCallback extends Callback {
        void invoke(Pointer window);
    }

    int proxyShouldClose(Pointer window);
    int proxyGetWidth(Pointer window);
    int proxyGetHeight(Pointer window);
    boolean proxyGetDestroyed(Pointer window);
    void proxyRelease(Pointer window);
    Pointer proxyGetNew();
    void proxyDrawEvent(Pointer window);
    void proxyIdleEvent(Pointer window);
    void proxyReshapeEvent(Pointer window, int width, int height);
    void proxyVisibleEvent(Pointer window, int vis);
    void proxyCreateEvent(Pointer window);
    void proxyCloseEvent(Pointer window);
    void proxyKeyEvent(Pointer window, byte k, int x, int y, int mods);
    void proxyClickEvent(Pointer window, int button, int state, int x, int y, int mods);
    void proxyMotionEvent(Pointer window, int x, int y);
    void proxySetSwapBuffersCallback(Pointer window, SwapBuffersCallback cb);
    void proxySetWindowTitleCallback(Pointer window, SetWindowTitleCallback cb);
    void proxySetRefreshWindowCallback(Pointer window, RefreshWindowCallback cb);
    void proxySetCreateWindowCallback(Pointer window, CreateWindowCallback cb);
}

/// Viewer class, basically an jogl canvas with the appropriate callbacks router to ProxyManager.
public class Viewer extends JFrame implements GLEventListener, WindowListener, MouseListener, MouseMotionListener, MouseWheelListener, KeyListener {
    Proxy proxy_;
    Pointer pointer_;
    GLCanvas canvas_;
    Proxy.SetWindowTitleCallback title_cb_;
    Proxy.RefreshWindowCallback refresh_cb_;
    Proxy.CreateWindowCallback create_cb_;
    int framebuffer_width_, framebuffer_height_;

    public Viewer(long pointer) {
        final File f = new File(Viewer.class.getProtectionDomain().getCodeSource().getLocation().getPath());
        final String sopath = f.getParent() + File.separator + ".." + File.separator + ".." + File.separator + "lib";
        //System.setProperty("jna.library.path", sopath);

		try {
           NativeLibrary.addSearchPath("DIPviewer", new File(sopath).getCanonicalPath());
		} catch (Exception e) {
          System.out.println(e.toString());
        }

        proxy_ = (Proxy) Native.loadLibrary("DIPviewer", Proxy.class);

        pointer_ = new Pointer(pointer);

        // Just a default. Will get overwritten on reshape event.
        framebuffer_width_ = proxy_.proxyGetWidth(pointer_);
        framebuffer_height_ = proxy_.proxyGetHeight(pointer_);

        GLProfile profile = GLProfile.get(GLProfile.GL2);
        GLCapabilities capabilities = new GLCapabilities(profile);
        capabilities.setDoubleBuffered(true);

        canvas_ = new GLCanvas(capabilities);

        canvas_.addGLEventListener(this);
        canvas_.addMouseListener(this);
        canvas_.addMouseMotionListener(this);
        canvas_.addMouseWheelListener(this);
        canvas_.addKeyListener(this);
        this.addWindowListener(this);

        this.setTitle("SliceViewer");
        this.getContentPane().add(canvas_);
        this.setSize(proxy_.proxyGetWidth(pointer_), proxy_.proxyGetHeight(pointer_));
        this.setLocationRelativeTo(null);
        this.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        this.setVisible(true);
        this.setResizable(true);

        canvas_.requestFocusInWindow();

        title_cb_ = new Proxy.SetWindowTitleCallback() {
            public void invoke(String title) {
                setTitle(title);
            }
        };
        proxy_.proxySetWindowTitleCallback(pointer_, title_cb_);

        refresh_cb_ = new Proxy.RefreshWindowCallback() {
            public void invoke() {
                canvas_.display();
            }
        };
        proxy_.proxySetRefreshWindowCallback(pointer_, refresh_cb_);

        create_cb_ = new Proxy.CreateWindowCallback() {
            public void invoke(Pointer window) {
                new Viewer(Pointer.nativeValue(window));
            }
        };
        proxy_.proxySetCreateWindowCallback(pointer_, create_cb_);
    }

    /// Convert from window to framebuffer coordinates
    Point translateMouse(MouseEvent e)
    {
        return new Point(e.getX() * framebuffer_width_ / canvas_.getWidth(),
                         e.getY() * framebuffer_height_ / canvas_.getHeight());
    }

    int translateModifiers(InputEvent e)
    {
      int m = e.getModifiersEx();
      int t = ((m&InputEvent.SHIFT_DOWN_MASK)>0?1:0) +
              ((m&InputEvent.CTRL_DOWN_MASK )>0?2:0) +
              ((m&InputEvent.ALT_DOWN_MASK  )>0?4:0) +
              ((m&InputEvent.META_DOWN_MASK )>0?8:0);
      return t;
    }

    public String display() {
        // Avoid error message when Matlab tries to disp() this object.
        return "org.java.viewer.Viewer";
    }

    public long[] pointer() {
        long[] retval = { pointer_.nativeValue( pointer_ ) };
        return retval;
    }

    // GLEventListener

    public void display(GLAutoDrawable drawable) {
        proxy_.proxyDrawEvent(pointer_);

        if (proxy_.proxyGetDestroyed(pointer_))
            dispose();
    }

    public void dispose(GLAutoDrawable drawable) {
    }

    public void init(GLAutoDrawable drawable) {
        drawable.getGL().setSwapInterval(0);
        proxy_.proxyCreateEvent(pointer_);
    }

    public void reshape(GLAutoDrawable drawable, int x, int y, int width, int height) {
        framebuffer_width_ = width;
        framebuffer_height_ = height;
        proxy_.proxyReshapeEvent(pointer_, width, height);
    }

    // MouseListener

    public void mouseClicked(MouseEvent e) {
    }

    public void mouseEntered(MouseEvent e) {
    }

    public void mouseExited(MouseEvent e) {
    }

    public void mousePressed(MouseEvent e) {
        final Point p = translateMouse(e);
        proxy_.proxyClickEvent(pointer_, e.getButton()-1, 0, p.x, p.y, translateModifiers(e));
    }

    public void mouseReleased(MouseEvent e) {
        final Point p = translateMouse(e);
        proxy_.proxyClickEvent(pointer_, e.getButton()-1, 1, p.x, p.y, translateModifiers(e));
    }

    // MouseMotionListener

    public void mouseMoved(MouseEvent e) {
    }

    public void mouseDragged(MouseEvent e) {
        final Point p = translateMouse(e);
        proxy_.proxyMotionEvent(pointer_, p.x, p.y);
    }

    // MouseWheelListener

    public void mouseWheelMoved(MouseWheelEvent e) {
        final Point p = translateMouse(e);

        int button = 3;
        if (e.getWheelRotation() > 0)
            button = button + 1;

        if (e.getWheelRotation() != 0)
        {
          proxy_.proxyClickEvent(pointer_, button, 1, p.x, p.y, translateModifiers(e));
          proxy_.proxyClickEvent(pointer_, button, 0, p.x, p.y, translateModifiers(e));
        }
    }

    // WindowListener

    public void windowActivated(WindowEvent e) {
    }

    public void windowClosed(WindowEvent e) {
        proxy_.proxyCloseEvent(pointer_);
        proxy_.proxyRelease(pointer_);
    }

    public void windowClosing(WindowEvent e) {
    }

    public void windowDeactivated(WindowEvent e) {
    }

    public void windowDeiconified(WindowEvent e) {
        proxy_.proxyVisibleEvent(pointer_, 1);
    }

    public void windowIconified(WindowEvent e) {
        proxy_.proxyVisibleEvent(pointer_, 0);
    }

    public void windowOpened(WindowEvent e) {
    }

    // KeyListener

    public void keyTyped(KeyEvent e) {
    }

    public void keyPressed(KeyEvent e) {
        int c = e.getKeyChar();

        // Control-Char
        if (c < 27)
            c = c + 'A' - 1;

        // use capitals
        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';

        int t = translateModifiers(e);
        if ((t & 3) == 3 && c == 'W')
        {
          // Ugly hack to avoid Matlab locking up when mass-closing windows
          System.out.println("Control-Shift-W not supported under Java");
        }
        else
          proxy_.proxyKeyEvent(pointer_, (byte)c, 0, 0, translateModifiers(e));

        if (proxy_.proxyGetDestroyed(pointer_))
            dispose();
    }

    public void keyReleased(KeyEvent e) {
    }
}
