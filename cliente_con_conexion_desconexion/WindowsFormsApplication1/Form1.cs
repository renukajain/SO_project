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
        bool conect;
        bool logIn;
        FormConsulta consulta;

        ClassPartida listP = new ClassPartida();

        delegate void delegado(string mensage);

        public Form1()
        {
            InitializeComponent();
            groupBox1.Visible = false;
            opcionesToolStripMenuItem.Visible = false;
            listBox1.Visible = false;
            label7.Visible = false;
            button6.Visible = false;
            conect = false;
            logIn = false;
        }
        class Limpiar{
            public void BorrarTextBox(Control control, GroupBox gb){
                foreach (var txt in control.Controls){
                    if (txt is TextBox)
                        ((TextBox)txt).Clear();
                }
                foreach (var combo in gb.Controls){
                    if (combo is ComboBox)
                        ((ComboBox)combo).SelectedIndex = 0;
                }
            }
        }

        private void mostrarLista(string nombres)//actualiza lista de conectados
        {
            string[] trozos = nombres.Split(' ');
            listBox1.Items.Clear();
            listBox1.BeginUpdate();
            for (int x = 0; x < trozos.Length-1; x++)
                listBox1.Items.Add(trozos[x]);
            listBox1.EndUpdate();
        }

        private void enviarConfi(string anfitrion, int partida)//pregunta si quiere jugar la partida y envia respuesta a server
        {
            string mensaje;
            string caption = anfitrion + " te esta invitando a la partida "+ partida;
            switch (MessageBox.Show("Aceptas la invitación?", caption, MessageBoxButtons.YesNo, MessageBoxIcon.Question))
            {
                case DialogResult.Yes:
                    mensaje = "/2"; //para confirmar que se ha respondido y aceptado
                    break;
                default:
                    mensaje = "/1";//para para confirmar que se ha respondido pero NO aceptado
                    break;
            }
            byte[] msg = System.Text.Encoding.ASCII.GetBytes("10/"+partida+mensaje);
            server.Send(msg);
        }

        private void setMssg(string mensaje) //funcioncion delegada de notificar
        { label8.Text=mensaje; }

        private void abrirTablero(int partida, int miFicha, int dim)//FUNCION DELEGADA DE ABRIR EL FORMULARIO PARTIDA
        {
            FormPartida p = new FormPartida(server, partida, miFicha, dim);
            listP.Guardar(p, partida);
            p.ShowDialog();
        }

        private void AtenderServidor(){
            while (true){
                //Recibimos la respuesta del servidor
                byte[] msg2 = new byte[80];
                server.Receive(msg2);
                string mensaje = Encoding.ASCII.GetString(msg2).Split('\0')[0];
                string [] trozos = mensaje.Split('/');
                string notificacion;
                
                try
                {
                    int cod = Convert.ToInt16(trozos[0]);
                    switch (cod){
                        case 1: //sign in
                            //Recibimos la respuesta del servidor
                            if (trozos[1] == "-1")
                                MessageBox.Show("existe usuario con este nombre");
                            else if (trozos[1] == "0")
                                MessageBox.Show(trozos[3]+" añadid@ con id "+trozos[2]);
                            break;
                        case 2://log in
                            if (trozos[1] == "-1")
                                MessageBox.Show("lista llena");
                            else if (trozos[1] == "-2")
                                MessageBox.Show("contraseña erronea");
                            else if (trozos[1] == "-3")
                                MessageBox.Show("no existe user");
                            else
                                logIn = true;
                                break;
                        case 3://consultas
                                consulta.dameRespuesta(trozos);//enviamos respuesta al formulario
                            break;
                        case 4:
                            listP.Recuperar(Convert.ToInt16(trozos[1])).formulario.recibirJugada(trozos[2]);
                            break;
                        case 5:
                            listP.Recuperar(Convert.ToInt16(trozos[1])).formulario.recibirGanador(trozos[2]);
                            break;
                        case 6://mostrar actualizar lista conectados
                            delegado del4 = new delegado(mostrarLista);
                            listBox1.Invoke(del4, new object[] { trozos[1] });
                            break;
                        case 7://error darse de baja
                            if (trozos[1] == "-1")
                                MessageBox.Show("Error. No te has podido dar de baja");
                            break;
                        case 8:
                            
                            break;
                        case 9://invitacion
                            notificacion="has sido invitado a partida id "+trozos[1]+" por "+trozos[2];
                            delegado del = new delegado(setMssg);
                            label8.Invoke(del, new object[] { notificacion });
                            enviarConfi(trozos[2], Convert.ToInt16(trozos[1]));
                            break;
                        case 10://partida aceptada
                            if (trozos[1] == "0"){
                                notificacion = "Todos han aceptado la partida " + trozos[2];
                                ThreadStart ts = delegate { abrirTablero(Convert.ToInt16(trozos[2]), Convert.ToInt16(trozos[3]), Convert.ToInt16(trozos[4])); };
                                Thread T = new Thread(ts);
                                T.Start();
                            }
                            else
                                notificacion = "alguien no ha aceptado la partida " + trozos[1];
                            delegado del1 = new delegado(setMssg);
                            label8.Invoke(del1, new object[] { notificacion });
                            break;
                        case 11:

                            listP.Recuperar(Convert.ToInt16(trozos[1])).formulario.recibirChat (trozos[2]);
                            break;
                    }
                }
                catch (FormatException){ }
            }
        }

        private void conectar()//establecer conexion
        {
            //Creamos un IPEndPoint con el ip del servidor y puerto del servidor al que deseamos conectarnos    
            IPAddress direc = IPAddress.Parse("147.83.117.22");
            IPEndPoint ipep = new IPEndPoint(direc, 50072);
           
            //Creamos el socket 
            server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                server.Connect(ipep);//Intentamos conectar el socket
                this.BackColor = Color.Green;
                MessageBox.Show("Conectado");
                conect = true;
                conectarToolStripMenuItem.Text = "Desconectar";
                groupBox1.Visible = true;
                groupBoxState(false);
                listBox1.Visible = true;
                label7.Visible = true;
                button6.Visible = true;
                opcionesToolStripMenuItem.Visible = true;
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

        private void signIn()//sing in
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

        private void desconectar()//desconexion
        {
            Limpiar limpiar = new Limpiar();
            if (conect == false)
                MessageBox.Show("NO HAY CONEXION");
            else{
                //Mensaje de desconexión
                byte[] msg = System.Text.Encoding.ASCII.GetBytes("0");
                server.Send(msg);

                // Nos desconectamos
                atender.Abort();
                this.BackColor = Color.Gray;
                server.Shutdown(SocketShutdown.Both);
                server.Close();
                conect = false;
                conectarToolStripMenuItem.Text = "Conectar";
                limpiar.BorrarTextBox(this, groupBox1);
            }
        }

        private void logIn_SendServer()//log in
        {
            string mensaje = "2/" + Nombre.Text + "/" + passwBox.Text;
            // Enviamos al servidor el nombre tecleado
            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);
        }

        private void button6_Click(object sender, EventArgs e){//enviamos peticion invitacion a partida
            if (logIn == true)
            {
                int contador = 0;
                string invitados = "/";
                foreach (var item in listBox1.SelectedItems)
                {
                    contador++;
                    string[] nom = listBox1.GetItemText(item).Split('\n');
                    invitados += nom[0] + ",";
                }
                string mensaje = "9/" + contador + invitados;
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
            else
                MessageBox.Show("primero debes iniciar sesion");
        }

        private void conectarToolStripMenuItem_Click(object sender, EventArgs e){
            if (conect == false)
                conectar();
            else
                desconectar();
        }

        private void hacerConsultaToolStripMenuItem_Click(object sender, EventArgs e){
            if (conect == true){
                consulta = new FormConsulta(server);
                consulta.ShowDialog();
            }else
                MessageBox.Show("no hay conexion");
        }

        private void groupBoxState(bool a){//modifica formato del groupBox segun se desee iniciar sesion o darse de alta
            if (a == false){//a=false modo logIN   a=true modo signIN
                groupBox1.Text = "Log In";
                linkLabel1.Text = "Sing In";
            }else{
                groupBox1.Text = "Signg In";
                linkLabel1.Text = "Log In";
            }
            label2.Visible = a;
            label5.Visible = a;
            comppasswBox.Visible = a;
            tBedad.Visible = a;
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e){//
            if (groupBox1.Text == "Log In")
                groupBoxState(true);
            else
                groupBoxState(false);          
        }

        private void button2_Click(object sender, EventArgs e){
            if (conect == true)
            {
                if (groupBox1.Text == "Log In")
                    logIn_SendServer();
                else
                    signIn();
            }
            else
                MessageBox.Show("primero debes conectarte");
        }

       private void unsubscribeToolStripMenuItem_Click(object sender, EventArgs e){//envia peticion de darse de baja
            if (logIn == true){
                string mensaje = "7/";
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
                atender.Abort();
                this.BackColor = Color.Gray;
                server.Shutdown(SocketShutdown.Both);
                server.Close();
                conect = false;
                logIn = false;
                conectarToolStripMenuItem.Text = "Conectar";

            }else
                MessageBox.Show("primero debes iniciar sesion");
        }
    }
}