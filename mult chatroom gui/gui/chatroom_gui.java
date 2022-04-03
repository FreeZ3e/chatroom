import com.formdev.flatlaf.FlatDarkLaf;
import com.formdev.flatlaf.FlatLightLaf;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.io.File;


public class chatroom_gui {
    public static void main(String[] args) {
        gui client = new gui();
        client.create_address_window();
    }
}

class gui_base
{
    final static protected String icon_path = "./chatroom_icon.png";
    final static protected String login_pic_path = "./login_pic.png";
    final static protected String lib_path = "./Socket_MultRoom_client_dll";

    //C++ lib load
    static
    {
        System.loadLibrary(lib_path);
    }

    //C++ API
    protected native long create_obj(String ip_address, int port_num);
    protected native int login(long ptr, String user_name, String passwd);
    protected native boolean send_msg(long ptr, String msg);
    protected native String recv_msg(long ptr);
    protected native String recv_history(long ptr);
    protected native String recv_online_user(long ptr);
    protected native boolean file_send(long ptr, String command);
    protected native boolean file_path_set(long ptr, String command);
    protected native void send_msg_to(long ptr, String command);
    protected native void delete_account(long ptr, String command);
    protected native void reset_passwd(long ptr, String command);

    //pointer of chatroom obj
    long obj_ptr;
}

class gui extends gui_base
{
    private JFrame address_window;
    private JPanel address_panel;
    private JTextField address_text;
    private JTextField port_text;
    private ActionListener address_listener;

    private JFrame login_window;
    private JPanel login_panel;
    private ActionListener login_listener;
    private JTextField user_name_text;
    private JPasswordField passwd_text;

    private JFrame main_window;
    private ActionListener send_listener;
    private ActionListener main_listener;
    private Action input_text_action;
    private JTextArea input_text;
    private JTextArea output_text;
    private JTextArea user_text;

    private String name;
    private String history_msg;
    private String online_user;


    public gui()
    {
        //gui style set
        FlatLightLaf.install();
        try {
            UIManager.setLookAndFeel( new FlatDarkLaf());
        } catch( Exception ex ) {
            System.err.println( "Failed to initialize LaF" );
        }

        user_name_text = new JTextField();
        passwd_text = new JPasswordField();
        address_text = new JTextField();
        port_text = new JTextField();

        //set ip address and port number
        address_listener = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String ip_address = address_text.getText();
                String port_string = port_text.getText();

                if(ip_address.isEmpty() || port_string.isEmpty())
                    JOptionPane.showMessageDialog(address_window,"address or port can't be empty" , "connect", JOptionPane.WARNING_MESSAGE);
                else {
                    int port_num = Integer.valueOf(port_string);
                    obj_ptr = create_obj(ip_address, port_num);

                    address_window.dispose();
                    create_login_window();
                }
            }
        };

        //check & create main_window
        //-1 : connect lost
        //0 : verify success
        //1 : passwd wrong
        //2 : user not exist
        //3 : user are login
        login_listener = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String s_user_name = user_name_text.getText();
                String s_passwd = passwd_text.getText();

                if(s_user_name.isEmpty() || s_passwd.isEmpty())
                {
                    JOptionPane.showMessageDialog(login_window, "name or passwd can't be empty", "login", JOptionPane.WARNING_MESSAGE);
                }
                else
                {
                    int res = login(obj_ptr, user_name_text.getText(), passwd_text.getText());

                    if (res == 0 || res == 2)
                    {
                        JOptionPane.showMessageDialog(login_window, "verify success", "login", JOptionPane.WARNING_MESSAGE);
                        name = user_name_text.getText();

                        login_window.dispose();
                        chat();
                    }
                    else if (res == 1)
                    {
                        JOptionPane.showMessageDialog(login_window, "passwd wrong", "login", JOptionPane.WARNING_MESSAGE);
                    }
                    else if (res == 3)
                    {
                        JOptionPane.showMessageDialog(login_window, "user are login", "login", JOptionPane.WARNING_MESSAGE);
                    }
                    else if (res == -1)
                    {
                        JOptionPane.showMessageDialog(login_window, "connect lost", "login", JOptionPane.WARNING_MESSAGE);
                        System.exit(-1);
                    }
                }
            }
        };

        //send function for main_window
        send_listener = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                boolean connect = send_msg(obj_ptr, input_text.getText());

                if(connect)
                {
                    output_text.append(name + ": ");
                    output_text.append(input_text.getText());
                    output_text.append("\n\n");
                    input_text.setText(null);

                    output_text.setCaretPosition(output_text.getDocument().getLength());
                }
                else
                {
                    JOptionPane.showMessageDialog(main_window, "connect lost", "chatroom", JOptionPane.WARNING_MESSAGE);
                    System.exit(-1);
                }
            }
        };

        //main window clicks function
        main_listener = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String command = e.getActionCommand();

                if(command.equals("file_choose_ack"))
                {
                    JFileChooser file_chooser = new JFileChooser();
                    file_chooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
                    file_chooser.showOpenDialog(null);

                    File file = file_chooser.getSelectedFile();

                    String _path = file.getAbsolutePath();
                    String path = _path.replaceAll("\\\\","/");
                    String target_name = JOptionPane.showInputDialog(main_window,"input the user");

                    while(target_name.isEmpty())
                    {
                        JOptionPane.showMessageDialog(main_window,"user can't be empty","chatroom", JOptionPane.WARNING_MESSAGE);
                        target_name = JOptionPane.showInputDialog(main_window,"input the user");
                    }

                    String msg = "/filesd " + target_name + " " + path;
                    if(file_send(obj_ptr, msg))
                        JOptionPane.showMessageDialog(main_window, "send success", "chatroom", JOptionPane.WARNING_MESSAGE);
                    else
                        JOptionPane.showMessageDialog(main_window, "send failed", "chatroom", JOptionPane.WARNING_MESSAGE);
                }
                else if(command.equals("file_path_ack"))
                {
                    JFileChooser path_chooser = new JFileChooser();
                    path_chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
                    path_chooser.showOpenDialog(null);

                    File file = path_chooser.getSelectedFile();

                    String _path = file.getAbsolutePath();
                    String path = _path.replaceAll("\\\\","/");

                    String msg = "/filept " + path;
                    if(file_path_set(obj_ptr, msg))
                        JOptionPane.showMessageDialog(main_window, "path set success", "chatroom", JOptionPane.WARNING_MESSAGE);
                    else
                        JOptionPane.showMessageDialog(main_window, "path set failed", "chatroom", JOptionPane.WARNING_MESSAGE);
                }
                else if(command.equals("send_to_ack"))
                {
                    String target_name = JOptionPane.showInputDialog(main_window,"input the user");

                    while(target_name.isEmpty())
                    {
                        JOptionPane.showMessageDialog(main_window, "user can't be empty", "chatroom", JOptionPane.WARNING_MESSAGE);
                        target_name = JOptionPane.showInputDialog(main_window, "input the user");
                    }

                    String msg = "/p " + target_name + "#" + input_text.getText();

                    send_msg_to(obj_ptr, msg);

                    output_text.append(name + ": ");
                    output_text.append(input_text.getText());
                    output_text.append("\n");
                    input_text.setText(null);
                }
                else if(command.equals("del_acc_ack"))
                {
                    delete_account(obj_ptr, "/accdel");
                    System.exit(0);

                }
                else if(command.equals("reset_passwd_ack"))
                {
                    String new_passwd = JOptionPane.showInputDialog(main_window,"input the new passwd");

                    while(new_passwd.isEmpty())
                    {
                        JOptionPane.showMessageDialog(main_window, "passwd can't be empty", "chatroom", JOptionPane.WARNING_MESSAGE);
                        new_passwd = JOptionPane.showInputDialog(main_window, "input the user");
                    }

                    String msg = "/passwd " + new_passwd;

                    reset_passwd(obj_ptr, msg);
                }
            }
        };

        //input_text enter action
        input_text_action = new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                input_text.setText(input_text.getText() + "\n");
            }
        };
    }

    public void create_address_window()
    {
        address_window = new JFrame("connect");
        address_window.setSize(300,200);
        address_window.setLocationRelativeTo(null);
        address_window.setResizable(false);
        address_window.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);

        Image image = Toolkit.getDefaultToolkit().getImage(icon_path);
        address_window.setIconImage(image);

        address_panel = new JPanel(new FlowLayout(FlowLayout.RIGHT));

        JLabel address_label = new JLabel("server ip");
        JLabel port_label = new JLabel("port number");
        JButton connect_btn = new JButton("connect");

        connect_btn.registerKeyboardAction(address_listener, KeyStroke.getKeyStroke(KeyEvent.VK_ENTER,0),JComponent.WHEN_IN_FOCUSED_WINDOW);
        connect_btn.addActionListener(address_listener);

        address_text.setPreferredSize(new Dimension(200,30));
        port_text.setPreferredSize(new Dimension(200,30));

        address_panel.add(address_label);
        address_panel.add(address_text);
        address_panel.add(port_label);
        address_panel.add(port_text);
        address_panel.add(connect_btn);

        address_window.setContentPane(address_panel);
        address_window.setVisible(true);
    }

    private void create_login_window()
    {
        login_window = new JFrame("login");
        login_window.setSize(400,400);
        login_window.setLocationRelativeTo(null);
        login_window.setResizable(false);
        login_window.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        login_window.getContentPane().setLayout(null);

        Image image = Toolkit.getDefaultToolkit().getImage(icon_path);
        login_window.setIconImage(image);

        login_panel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        login_panel.setBounds(0,0,300,400);

        JLabel name_label = new JLabel("user name");
        JLabel passwd_label = new JLabel("password");
        JLabel pic_label = new JLabel(new ImageIcon(login_pic_path));
        name_label.setForeground(Color.BLACK);
        passwd_label.setForeground(Color.BLACK);
        pic_label.setBounds(0,0,400,400);

        JButton login_btn = new JButton("login");
        login_btn.registerKeyboardAction(login_listener, KeyStroke.getKeyStroke(KeyEvent.VK_ENTER,0),JComponent.WHEN_IN_FOCUSED_WINDOW);
        login_btn.addActionListener(login_listener);

        user_name_text.setPreferredSize(new Dimension(200,30));
        passwd_text.setPreferredSize(new Dimension(200,30));

        login_panel.add(name_label);
        login_panel.add(user_name_text);
        login_panel.add(passwd_label);
        login_panel.add(passwd_text);
        login_panel.add(login_btn);

        //set background
        login_window.getLayeredPane().add(pic_label, new Integer(Integer.MIN_VALUE));

        JPanel j=(JPanel)login_window.getContentPane();
        j.setOpaque(false);
        login_panel.setOpaque(false);

        login_window.add(login_panel);
        login_window.setVisible(true);
    }

    private void chat()
    {
        //init
        recv_init_msg();

        create_main_window();

        set_init_msg();
        recv_listen();
    }

    private void create_main_window()
    {
        //window set

        main_window = new JFrame("chatroom");
        main_window.setSize(800,800);
        main_window.setLocationRelativeTo(null);
        main_window.setResizable(false);
        main_window.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        main_window.getContentPane().setLayout(null);

        Image image = Toolkit.getDefaultToolkit().getImage(icon_path);
        main_window.setIconImage(image);

        //text area

        input_text = new JTextArea();
        output_text = new JTextArea();
        user_text = new JTextArea();

        input_text.getInputMap().put(KeyStroke.getKeyStroke("ENTER"),"none");   //remove the hotkey
        input_text.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, InputEvent.CTRL_DOWN_MASK), "enterAction");  //bind new hotkey & action
        input_text.getActionMap().put("enterAction", input_text_action);
        input_text.setLineWrap(true);
        input_text.setWrapStyleWord(true);

        output_text.setEditable(false);
        output_text.setLineWrap(true);
        output_text.setWrapStyleWord(true);

        user_text.setEditable(false);
        user_text.setPreferredSize(new Dimension(150,200));

        //button area

        //in panel

        JButton send_btn = new JButton("send");
        send_btn.registerKeyboardAction(send_listener, KeyStroke.getKeyStroke(KeyEvent.VK_ENTER,0),JComponent.WHEN_IN_FOCUSED_WINDOW);
        send_btn.addActionListener(send_listener);

        JButton file_choose_btn = new JButton("file send");
        file_choose_btn.setActionCommand("file_choose_ack");
        file_choose_btn.addActionListener(main_listener);

        JButton send_to_btn = new JButton("send to");
        send_to_btn.setActionCommand("send_to_ack");
        send_to_btn.addActionListener(main_listener);

        //in func_panel

        JButton file_path_btn = new JButton("save path");
        file_path_btn.setActionCommand("file_path_ack");
        file_path_btn.addActionListener(main_listener);

        JButton del_acc_btn = new JButton("del account");
        del_acc_btn.setActionCommand("del_acc_ack");
        del_acc_btn.addActionListener(main_listener);

        JButton reset_passwd_btn = new JButton("reset passwd");
        reset_passwd_btn.setActionCommand("reset_passwd_ack");
        reset_passwd_btn.addActionListener(main_listener);


        //scroll panel set

        JScrollPane output_scroll = new JScrollPane(output_text, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

        output_scroll.setBounds(10,10,400,500);

        JScrollPane input_scroll = new JScrollPane(input_text, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

        input_scroll.setBounds(10,600,500,100);

        //panel set

        JPanel panel = new JPanel(new FlowLayout(FlowLayout.LEFT));

        panel.add(send_btn);
        panel.add(send_to_btn);
        panel.add(file_choose_btn);
        panel.setBounds(510,600,800,300);

        //func_panel set

        JPanel func_panel = new JPanel();

        func_panel.add(file_path_btn);
        func_panel.add(del_acc_btn);
        func_panel.add(reset_passwd_btn);
        func_panel.setBounds(400,300,400,400);

        //user_scroll set

        JScrollPane user_scroll = new JScrollPane(user_text, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

        user_scroll.setBounds(450,10,150,200);


        //main_window set

        main_window.add(output_scroll);
        main_window.add(user_scroll);
        main_window.add(input_scroll);
        main_window.add(panel);
        main_window.add(func_panel);
        main_window.setVisible(true);

        //text set

        input_text.requestFocus();
        user_text.setText("online user\n");
    }

    private void recv_listen()
    {
        Thread t = new Thread(new Runnable() {
            @Override
            public void run() {
                while(true)
                {
                    String msg = recv_msg(obj_ptr);

                    if(msg.equals("error"))//lost connect
                    {
                        JOptionPane.showMessageDialog(login_window, "connect lost", "chatroom", JOptionPane.WARNING_MESSAGE);
                        System.exit(-1);
                    }
                    else if(msg.matches("server:(.*)"))//server msg
                    {
                        if(msg.matches("(.*)is online"))
                        {
                            String res = msg.substring(msg.indexOf(": ") + 2,msg.indexOf(" is online"));
                            user_text.append(res + "\n");
                        }
                        else if(msg.matches("(.*)is disconnected"))
                        {
                            String logout_name = msg.substring(msg.indexOf(": ") + 2,msg.indexOf(" is disconnected"));
                            String[] user_list = user_text.getText().split("\n");
                            StringBuilder sb = new StringBuilder();

                            for (int i = 0; i < user_list.length; i++) {
                                if(!user_list[i].equals(logout_name))
                                    sb.append(user_list[i] + "\n");
                            }

                            user_text.setText(sb.toString());
                        }
                        else
                        {
                            output_text.append("        " + msg + "\n\n");
                            output_text.setCaretPosition(output_text.getDocument().getLength());
                        }
                    }
                    else
                    {
                        output_text.append("        " + msg + "\n\n");
                        output_text.setCaretPosition(output_text.getDocument().getLength());
                    }
                }
            }
        });

        t.start();
    }

    private void recv_init_msg()
    {
        history_msg = recv_history(obj_ptr);
        send_msg(obj_ptr,"/ack");
        online_user = recv_online_user(obj_ptr);
    }

    private void set_init_msg()
    {
        if(!history_msg.equals("/hisend"))
        {
            output_text.append(history_msg);
            output_text.append("------------------------history here----------------------\n");
        }

        if(!online_user.equals("/urend"))
        {
            String[] online_user_list = online_user.split(" is online\n");

            for(int n=0;n<online_user_list.length;++n)
                user_text.append(online_user_list[n] + "\n");
        }
    }
}