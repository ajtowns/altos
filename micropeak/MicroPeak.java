/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package org.altusmetrum.micropeak;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.AltosLib.*;

public class MicroPeak extends JFrame implements ActionListener, ItemListener {

	File		filename;
	MicroGraph	graph;
	MicroData	data;
	Container	pane;

	private void OpenFile(File filename) {
		try {
			FileInputStream	input = new FileInputStream(filename);
			try {
				data = new MicroData(input);
				graph = new MicroGraph(data);
				pane.add(graph.panel);
			} catch (IOException ioe) {
			}
			try {
				input.close();
			} catch (IOException ioe) {
			}
		} catch (FileNotFoundException fne) {
		}
	}

	private void SelectFile() {
	}

	private void DownloadData() {
		java.util.List<MicroUSB>	devices = MicroUSB.list();
		for (MicroUSB device : devices)
			System.out.printf("device %s\n", device.toString());
	}

	public void actionPerformed(ActionEvent ev) {
		System.out.printf("action %s %s\n", ev.getActionCommand(), ev.paramString());
		if ("Exit".equals(ev.getActionCommand()))
			System.exit(0);
		else if ("Open".equals(ev.getActionCommand()))
			SelectFile();
		else if ("New".equals(ev.getActionCommand()))
			new MicroPeak();
		else if ("Download".equals(ev.getActionCommand()))
			DownloadData();
	}

	public void itemStateChanged(ItemEvent e) {
	}

	public MicroPeak(File filename) {

		this.filename = filename;

		pane = getContentPane();

//		JLabel label = new JLabel ("Hello, World");
//		pane.add(label);

		setSize(800, 500);

		setTitle("MicroPeak");

		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);

		JMenu fileMenu = new JMenu("File");
		menuBar.add(fileMenu);

		JMenuItem newAction = new JMenuItem("New");
		fileMenu.add(newAction);
		newAction.addActionListener(this);

		JMenuItem openAction = new JMenuItem("Open");
		fileMenu.add(openAction);
		openAction.addActionListener(this);

		JMenuItem downloadAction = new JMenuItem("Download");
		fileMenu.add(downloadAction);
		downloadAction.addActionListener(this);

		JMenuItem exitAction = new JMenuItem("Exit");
		fileMenu.add(exitAction);
		exitAction.addActionListener(this);

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				System.exit(0);
			}
		});

		if (filename != null)
			this.OpenFile(filename);
		setVisible(true);
	}

	public MicroPeak() {
		this(null);
	}

	public static void main(final String[] args) {
		boolean	opened = false;

		for (int i = 0; i < args.length; i++) {
			new MicroPeak(new File(args[i]));
			opened = true;
		}
		if (!opened)
			new MicroPeak();
	}
}