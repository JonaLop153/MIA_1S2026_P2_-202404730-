import { Component } from '@angular/core';
import { Router } from '@angular/router';
import { AuthContext } from '../../context/AuthContext';

@Component({
  selector: 'app-toolbar',
  templateUrl: './toolbar.component.html',
  styleUrls: ['./toolbar.component.css']
})
export class ToolbarComponent {
  usuarioLogueado: string = '';

  constructor(
    private router: Router,
    private auth: AuthContext
  ) {
    this.usuarioLogueado = this.auth.getUsuario();
  }

  cerrarSesion() {
    this.auth.cerrarSesion();
    this.router.navigate(['/login']);
  }

  irAHome() {
    this.router.navigate(['/dashboard']);
  }

  irAExplorer() {
    this.router.navigate(['/explorer']);
  }

  irAJournaling() {
    this.router.navigate(['/journaling']);
  }
}