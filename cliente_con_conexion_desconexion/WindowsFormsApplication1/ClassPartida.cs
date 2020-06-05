using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WindowsFormsApplication1
{
    public class ClassPartida
    {
        public class Partida{
            public FormPartida formulario;
            public int id;
        }

        List<Partida> lista = new List<Partida>();

        public void Guardar(FormPartida f, int id)
        {
            Partida p = new Partida();
            p.formulario = f;
            p.id = id;
            lista.Add(p); 
        }

        public Partida Recuperar(int id)
        {
            for (int i = 0; i < lista.Count(); i++){
                if (lista[i].id==id)
                    return lista[i];
            }
            return null;
        }
        
        public void Eliminar(int id)
        {
            Partida p= Recuperar(id);
            lista.Remove(p);
        }
    }
}
