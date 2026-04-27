import { Component, OnInit } from '@angular/core';
import { BackendService } from '../../servicios/backend.service';
import { AuthContext } from '../../context/AuthContext';

@Component({
  selector: 'app-file-explorer',
  templateUrl: './file-explorer.component.html',
  styleUrls: ['./file-explorer.component.css']
})
export class FileExplorerComponent implements OnInit {
  rutaActual: string = '/';
  archivos: any[] = [];
  mensaje: string = '';

  constructor(
    private backend: BackendService,
    private auth: AuthContext
  ) {}

  ngOnInit() {
    this.cargarContenido();
  }

  cargarContenido() {
    if (!this.auth.getSesionActiva()) {
      this.mensaje = 'Debe iniciar sesión para explorar archivos';
      return;
    }

    const comando = `ls -path=${this.rutaActual}`;
    
    this.backend.ejecutarComando(comando).subscribe({
      next: (respuesta: any) => {
        this.mensaje = respuesta.respuesta;
      },
      error: (e) => {
        this.mensaje = 'Error al cargar contenido';
        console.error(e);
      }
    });
  }

  navegarCarpeta(nombre: string) {
    if (nombre === '..') {
      const partes = this.rutaActual.split('/').filter(p => p);
      partes.pop();
      this.rutaActual = '/' + partes.join('/');
    } else {
      this.rutaActual = this.rutaActual === '/' ? `/${nombre}` : `${this.rutaActual}/${nombre}`;
    }
    this.cargarContenido();
  }

  verContenidoArchivo(nombre: string) {
    const ruta = this.rutaActual === '/' ? `/${nombre}` : `${this.rutaActual}/${nombre}`;
    const comando = `cat -file1=${ruta}`;
    
    this.backend.ejecutarComando(comando).subscribe({
      next: (respuesta: any) => {
        alert(respuesta.respuesta);
      },
      error: (e) => {
        alert('Error al leer archivo');
        console.error(e);
      }
    });
  }
}