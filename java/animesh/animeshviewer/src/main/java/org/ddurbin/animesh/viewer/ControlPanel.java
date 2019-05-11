package org.ddurbin.animesh.viewer;

import org.ddurbin.common.Check;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class ControlPanel extends JPanel {
    public ControlPanel() {
        super();
        setLayout(new GridLayout(3,2));
        add( new Label("Normals"));
        JCheckBox cb1 = new JCheckBox();
        cb1.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println( e );
            }
        });
        add(cb1);
        add( new Label("Primary Tangent"));
        JCheckBox cb2 = new JCheckBox();
        cb2.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println( e );
            }
        });
        add(cb2);

        add( new Label("Other Tangents"));
        JCheckBox cb3 = new JCheckBox();
        cb3.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.out.println( e );
            }
        });
        add(cb3);
    }

}
