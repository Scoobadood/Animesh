package org.ddurbin.animesh.viewer;

import javax.swing.JCheckBox;
import javax.swing.JPanel;
import javax.swing.plaf.basic.BasicSplitPaneUI;

import java.awt.*;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

public class ControlPanel extends JPanel {
    private JoglMain jm;
    private JCheckBox cbEnableNormals;
    private JCheckBox cbEnablePrincipalTangent;
    private JCheckBox cbEnableAllTangents;

    public ControlPanel(JoglMain jm) {
        super(new GridLayout(4,1));
        this.jm = jm;
        cbEnableNormals = new JCheckBox("Enable normals", jm.isNormalsEnabled());
        cbEnableNormals.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if( e.getStateChange() == ItemEvent.SELECTED ) {
                    jm.enableNormals(true);
                } else {
                    jm.enableNormals(false);
                }
            }
        });
        add(cbEnableNormals);
        cbEnablePrincipalTangent = new JCheckBox("Enable principal tangent", jm.isPrincipalTangentEnabled());
        cbEnablePrincipalTangent.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if( e.getStateChange() == ItemEvent.SELECTED ) {
                    jm.enablePrincipalTangent(true);
                } else {
                    jm.enablePrincipalTangent(false);
                }
            }
        });
        add(cbEnablePrincipalTangent);
        cbEnableAllTangents = new JCheckBox("Enable all tangents", jm.isTangentsEnabled());
        cbEnableAllTangents.addItemListener(new ItemListener() {
            @Override
            public void itemStateChanged(ItemEvent e) {
                if( e.getStateChange() == ItemEvent.SELECTED ) {
                    jm.enableTangents(true);
                } else {
                    jm.enableTangents(false);
                }
            }
        });

        add(cbEnableAllTangents);
    }

}
