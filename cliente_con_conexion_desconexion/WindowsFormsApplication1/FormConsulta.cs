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
        { tBcons.Text = "Jugadores que jugaron con el usuario cuyo username tienes que indicar aquí"; }

        private void radioButton1_MouseLeave(object sender, EventArgs e)
        { tBcons.Text = ""; }

        private void radioButton2_MouseHover(object sender, EventArgs e)
        { tBcons.Text = "Ganadores de las partidas que has jugado con el usuario que indiques aquí"; }

        private void radioButton2_MouseLeave(object sender, EventArgs e)
        { tBcons.Text = ""; }

        private void radioButton3_MouseHover(object sender, EventArgs e)
        { tBcons.Text = "Partidas que se jugaron en el intervalo de tiempo que indiques abajo"; }

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
                        label1.Invoke(new delegado(escribirRes), new object[] { "Jugador(Partida): " + trozos[2] });
                        break;
                    case 4:
                        label1.Invoke(new delegado(escribirRes), new object[] { "Ganadores(Partida) " + trozos[2] });
                        break;
                    case 5:
                        label1.Invoke(new delegado(escribirRes), new object[] { "Partidas entre este intervalo " + trozos[2] });
                        break;
                }
            }
        }

        private void button5_Click(object sender, EventArgs e){
            if (radioButton3.Checked){// consulta 3 jusgadores que han perdido
                int tiempoI = (dTP1.Value.Year * 10000) + (dTP1.Value.Month * 100) + dTP1.Value.Day;
                int tiempoF = (dTP2.Value.Year * 10000) + (dTP2.Value.Month * 100) + dTP2.Value.Day;
                if (tiempoF - tiempoI < 0)
                    MessageBox.Show("Intervalo escogido no es adecuado.");
                else
                {
                    string mensaje = "5/" + tiempoI.ToString() + "/" + tiempoF.ToString();
                    // Enviamos al servidor el nombre tecleado
                    MessageBox.Show(mensaje);
                    byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                    server.Send(msg);
                }
            }
            else if (tBcons.Text == "")
                MessageBox.Show("introduce datos necesarios para realizar consulta");
            else if (radioButton1.Checked)
            {//CONSULTA 1 NOMBRE JUGADORES QUE HAN JUGADO CON
                string mensaje = "3/" + tBcons.Text;
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
            else if (radioButton2.Checked)
            {//consulta 2 ciudad en las que ha jugado "username"
                string mensaje = "4/" + tBcons.Text;
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
            else
                MessageBox.Show("selecciona cosulta");
        }

        private void timerSET(bool a){
            dTP1.Visible = a;
            dTP2.Visible = a;
            label2.Visible = a;
            label3.Visible = a;
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            timerSET(true);
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            timerSET(false);
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            timerSET(false);
        }

    }
}
