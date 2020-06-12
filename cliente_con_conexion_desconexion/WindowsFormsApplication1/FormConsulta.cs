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

namespace WindowsFormsApplication1
{
    public partial class FormConsulta : Form
    {
        Socket server;
        delegate void delegado(string mensage);
        public FormConsulta(Socket server)
        {
            InitializeComponent();
            this.server = server;
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
        { tBcons.Text = "Jugadores que jugaron con el usuario cuyo username tienes que indicar aquí"; }

        private void radioButton3_MouseLeave(object sender, EventArgs e)
        { tBcons.Text = ""; }

        private void escribirRes(string text){
            label1.Text=text;
        }

        public void dameRespuesta(string[] trozos) { 
            if (trozos[2]=="-1")
                label1.Invoke(new delegado(escribirRes), new object[] { "no hay resultado" });
            else if (trozos[2]=="-2")
                label1.Invoke(new delegado(escribirRes), new object[] { "no hay resultado"});
            else{
                switch (Convert.ToInt16(trozos[1])){
                    case 3:
                        label1.Invoke(new delegado(escribirRes), new object[] { "jugadores menores " + trozos[2] });
                        break;
                    case 4:
                        label1.Invoke(new delegado(escribirRes), new object[] { "ciudades " + trozos[2] });
                        break;
                    case 5:
                        label1.Invoke(new delegado(escribirRes), new object[] { "jugadores perdedores " + trozos[2] });
                        break;
                }
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            
            if (tBcons.Text == "")
                MessageBox.Show("introduce datos necesarios para realizar consulta");
            else
            {
                if (radioButton1.Checked)
                {//CONSULTA 1 NOMBRE JUGADORES MENORES DE EDAD
                    try
                    {
                        int id = (Convert.ToInt16(tBcons.Text));
                        string mensaje = "3/" + tBcons.Text;
                        // Enviamos al servidor el nombre tecleado
                        byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                        server.Send(msg);
                    }
                    catch (FormatException)
                    {
                        MessageBox.Show("formato campo id incorrecto");
                    }
                }
                else if (radioButton2.Checked)
                {//consulta 2 ciudad en las que ha jugado "username"
                    string mensaje = "4/" + tBcons.Text;
                    // Enviamos al servidor el nombre tecleado
                    byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                    server.Send(msg);
                }
                else if (radioButton3.Checked)
                {// consulta 3 jusgadores que han perdido
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
}
