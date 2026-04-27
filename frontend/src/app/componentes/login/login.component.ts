import { Component } from '@angular/core';
import { Router } from '@angular/router';
import { BackendService } from '../../servicios/backend.service';
import { AuthContext } from '../../context/AuthContext';

@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.css']
})
export class LoginComponent {
  usuario: string = '';
  password: string = '';
  idParticion: string = '';
  mensaje: string = '';
  error: string = '';

  constructor(
    private backend: BackendService,
    private router: Router,
    private auth: AuthContext
  ) {}

  iniciarSesion() {
    this.mensaje = '';
    this.error = '';

    if (!this.usuario || !this.password || !this.idParticion) {
      this.error = 'Todos los campos son obligatorios';
      return;
    }

    const comando = `login -user=${this.usuario} -pass=${this.password} -id=${this.idParticion}`;
    
    this.backend.ejecutarComando(comando).subscribe({
      next: (respuesta: any) => {
        if (respuesta.respuesta.includes('exitosamente')) {
          this.auth.setSesionActiva(true);
          this.auth.setUsuario(this.usuario);
          this.auth.setIdParticion(this.idParticion);
          this.mensaje = 'Sesión iniciada exitosamente';
          setTimeout(() => this.router.navigate(['/dashboard']), 1500);
        } else {
          this.error = respuesta.respuesta;
        }
      },
      error: (e) => {
        this.error = 'Error de conexión con el backend';
        console.error(e);
      }
    });
  }
}