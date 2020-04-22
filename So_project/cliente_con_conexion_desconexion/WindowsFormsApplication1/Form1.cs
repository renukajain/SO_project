using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        Socket server;
        Thread atender;
        bool conect = false;
        public Form1()
        {
            InitializeComponent();
            CheckForIllegalCrossThreadCalls = false;
        }

        private void AtenderServidor()
        {
            while (true)
            {
                //Recibimos la respuesta del servidor
                byte[] msg2 = new byte[80];
                server.Receive(msg2);
                string mensaje = Encoding.ASCII.GetString(msg2).Split('\0')[0];
                string [] trozos = mensaje.Split('/');
                try
                {
                    int cod = Convert.ToInt16(trozos[0]);
                    switch (cod)
                    {
                        case 1: //sign in
                            //Recibimos la respuesta del servidor
                            if (trozos[1] == "-1")
                                MessageBox.Show("existe usuario con este nombre");
                            else if (trozos[1] == "0")
                                MessageBox.Show(trozos[3]+" añadid@ con id "+trozos[2]);
                            break;
                        case 2://log in
                            if (trozos[1] == "0")
                                MessageBox.Show("ok");
                            else if (trozos[1] == "-1")
                                MessageBox.Show("lista llena");
                            else if (trozos[1] == "-2")
                                MessageBox.Show("contraseña erronea");
                            else if (trozos[1] == "-3")
                                MessageBox.Show("no existe user");
                            //else
                                //MessageBox.Show("¡Bienvenid@!");
                                break;
                        case 3:
                            if (trozos[1] == "-1")
                                MessageBox.Show("no hay resultado");
                            else if (trozos[1] == "-2")
                                MessageBox.Show("error consultae");
                            else
                                MessageBox.Show("Jugadores menores de edad en la partida:" + trozos[1]);
                            break;
                        case 4:
                            if (trozos[1] == "-1")
                                MessageBox.Show("no hay resultado");
                            else if (trozos[1] == "-2")
                                MessageBox.Show("error consultae");
                            else
                                MessageBox.Show("Ciudades donde ha jugado:" + trozos[1]);
                            break;
                        case 5:
                            if (trozos[1] == "-1")
                                MessageBox.Show("no hay resultado");
                            else if (trozos[1] == "-2")
                                MessageBox.Show("error consultae");
                            else
                                MessageBox.Show("Jugadores que han perdido:" + trozos[1]);
                            break;
                        case 6:
                            textBox1.Text = trozos[1];
                            break;
                        case 7:
                            button7.Text = "num peticiones: " + trozos[1];
                            break;
                        case 8:
                            if (trozos[1] == "-1")
                                MessageBox.Show("no hay usuarios conectados");
                            else
                                MessageBox.Show("lista" + trozos[1]);
                            break;
                    }
                }
                catch (FormatException) { }
            }
        }

        private void button1_Click(object sender, EventArgs e)//establecer conexion
        {
            //Creamos un IPEndPoint con el ip del servidor y puerto del servidor 
            //al que deseamos conectarnos
            IPAddress direc = IPAddress.Parse("192.168.56.101");
            IPEndPoint ipep = new IPEndPoint(direc, 9050);
            

            //Creamos el socket 
            server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                server.Connect(ipep);//Intentamos conectar el socket
                this.BackColor = Color.Green;
                MessageBox.Show("Conectado");
                conect = true;
            }
            catch (SocketException ex)
            {
                //Si hay excepcion imprimimos error y salimos del programa con return 
                MessageBox.Show("No he podido conectar con el servidor");
                return;
            }

            ThreadStart ts = delegate { AtenderServidor(); };
            atender  = new Thread (ts);
            atender.Start();

        }

        private void button2_Click(object sender, EventArgs e)//sing in
        {
            if (conect == false)
                MessageBox.Show("NO SE HA REALIZADO CONEXION");
            else if (Nombre.Text == "" || passwBox.Text == "" || tBedad.Text == "")
                MessageBox.Show("DATOS INCOMPLETOS");
            else{
                try{
                    int EDAD = Convert.ToInt16(tBedad.Text);
                    string mensaje = "1/" + Nombre.Text + "/" + passwBox.Text + "/" + tBedad.Text;
                    // Enviamos al servidor el nombre tecleado
                    if (passwBox.Text.Equals(comppasswBox.Text)){
                        byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                        server.Send(msg);
                    }else
                        MessageBox.Show("La contraseña no coincide");
                }catch (FormatException){
                    MessageBox.Show("FORMATO DEL CAMPO EDAD INCORRECTO");
                }
            }
        }
                        

        private void button3_Click(object sender, EventArgs e)//desconexion
        {
            if (conect == false)
                MessageBox.Show("NO HAY CONEXION");
            else
            {
                //Mensaje de desconexión
                string mensaje = "0";

                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);

                // Nos desconectamos
                atender.Abort();
                this.BackColor = Color.Gray;
                server.Shutdown(SocketShutdown.Both);
                server.Close();
                conect = false;
            }
        }

        private void button4_Click(object sender, EventArgs e)//log in
        {
            if (conect == false)
                MessageBox.Show("NO HAY CONEXION");
            else{
                string mensaje = "2/" + id_in.Text + "/" + passw_in.Text;
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
        }

        private void button5_Click(object sender, EventArgs e)//CONSUTAS
        {
            if (conect == false)
                MessageBox.Show("NO HAY CONEXION");
            else
            {
                if (tBcons.Text == "")
                    MessageBox.Show("introduce datos necesarios para realizar consulta");
                else
                {
                    if (radioButton1.Checked){//CONSULTA 1 NOMBRE JUGADORES MENORES DE EDAD
                        try{
                            int id = (Convert.ToInt16(tBcons.Text));
                            string mensaje = "3/" + tBcons.Text;
                            // Enviamos al servidor el nombre tecleado
                            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                            server.Send(msg);
                        }catch(FormatException){
                            MessageBox.Show("formato campo id incorrecto");
                        }
                    }
                    else if (radioButton2.Checked){//consulta 2 ciudad en las que ha jugado "username"
                        string mensaje = "4/" + tBcons.Text;
                        // Enviamos al servidor el nombre tecleado
                        byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                        server.Send(msg);
                    }
                    else if (radioButton3.Checked){// consulta 3 jusgadores que han perdido
                        string mensaje = "5/" + tBcons.Text;
                        // Enviamos al servidor el nombre tecleado
                        byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                        server.Send(msg);
                    }
                    else
                        MessageBox.Show("selecciona cosulta");
                }
            }

        }



        private void radioButton1_MouseHover(object sender, EventArgs e)
        { tBcons.Text = "Nombre de los jugadores menores de edad en partida cuyo id debes introducir aquí"; }

        private void radioButton1_MouseLeave(object sender, EventArgs e)
        { tBcons.Text = ""; }

        private void radioButton2_MouseHover(object sender, EventArgs e)
        { tBcons.Text = "Ciudades donde ha jugado el jugagado cuyo username debes indicar aquí"; }

        private void radioButton2_MouseLeave(object sender, EventArgs e)
        { tBcons.Text = ""; }

        private void radioButton3_MouseHover(object sender, EventArgs e)
        { tBcons.Text = "Jugadores que perdieron contra el usuario cuyo username tienes que indicar aquí"; }

        private void radioButton3_MouseLeave(object sender, EventArgs e)
        { tBcons.Text = ""; }

        private void button8_Click(object sender, EventArgs e)
        {
            if (conect == false)
                MessageBox.Show("NO SE HA REALIZADO CONEXION");
            else
            {
                string mensaje = "8";
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
        }
    }
}
