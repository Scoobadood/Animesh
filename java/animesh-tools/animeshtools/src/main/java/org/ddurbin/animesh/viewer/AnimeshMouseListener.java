package org.ddurbin.animesh.viewer;

import com.jogamp.newt.event.MouseEvent;
import com.jogamp.newt.event.MouseListener;

public class AnimeshMouseListener implements MouseListener {
    private int mouseDownX;
    private int mouseDownY;
    private JoglMain jm;

    public AnimeshMouseListener( JoglMain jm) {
        this.jm = jm;
    }

    private boolean isShiftModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) != 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) == 0)
                && ((mod & MouseEvent.CTRL_MASK) == 0);
    }

    private boolean isNoModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) == 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) == 0)
                && ((mod & MouseEvent.CTRL_MASK) == 0);
    }

    private boolean isCmdModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) == 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) != 0)
                && ((mod & MouseEvent.CTRL_MASK) == 0);
    }

    private boolean isCtrlModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) == 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) == 0)
                && ((mod & MouseEvent.CTRL_MASK) != 0);
    }

    @Override
    public void mouseWheelMoved(MouseEvent e) {
    }
    @Override
    public void mouseClicked(MouseEvent e) {
    }

    @Override
    public void mouseEntered(MouseEvent e) {
    }

    @Override
    public void mouseExited(MouseEvent e) {
    }

    @Override
    public void mousePressed(MouseEvent e) {
        mouseDownX = e.getX();
        mouseDownY = e.getY();
    }

    @Override
    public void mouseDragged(MouseEvent e) {
        int deltaX = (e.getX() - mouseDownX);
        int deltaY = (e.getY() - mouseDownY);
        mouseDownX = e.getX();
        mouseDownY = e.getY();

        int mod = e.getModifiers();

        if (isNoModifier(mod)) {
            jm.rotateXY(deltaX, deltaY);
        } else if (isCmdModifier(mod)) {
            jm.rotateZ(deltaX);
        } else if (isCtrlModifier(mod)) {
            jm.zoom(deltaY*4);
        } else if (isShiftModifier(mod)) {
            jm.pan(deltaX, deltaY);
        }
    }

    @Override
    public void mouseReleased(MouseEvent e) {
    }

    @Override
    public void mouseMoved(MouseEvent e) {
    }}
