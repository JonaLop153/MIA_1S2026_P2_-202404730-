import { Component, OnInit } from '@angular/core';
import { BackendService } from '../../servicios/backend.service';
import { AuthContext } from '../../context/AuthContext';

@Component({
  selector: 'app-consola',
  templateUrl: './consola.component.html',
  styleUrls: ['./consola.component.css']
})
export class ConsolaComponent implements OnInit {
  comando: string = '';
  salida: string[] = [];

  constructor(
    private backend: BackendService,
    private auth: AuthContext
  ) {}

  ngOnInit() {}

  ejecutarComando() {
    if (!this.comando.trim()) return;

    this.salida.push(`> ${this.comando}`);

    this.backend.ejecutarComando(this.comando).subscribe({
      next: (respuesta: any) => {
        this.salida.push(respuesta.respuesta || 'Comando ejecutado');
        this.comando = '';
      },
      error: (e) => {
        this.salida.push('Error de conexión con el backend');
        console.error(e);
      }
    });
  }

  limpiarConsola() {
    this.salida = [];
  }
}