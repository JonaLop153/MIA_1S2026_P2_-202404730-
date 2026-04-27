import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class AuthContext {
  private sesionActiva: boolean = false;
  private usuario: string = '';
  private idParticion: string = '';

  setSesionActiva(valor: boolean) {
    this.sesionActiva = valor;
  }

  getSesionActiva(): boolean {
    return this.sesionActiva;
  }

  setUsuario(valor: string) {
    this.usuario = valor;
  }

  getUsuario(): string {
    return this.usuario;
  }

  setIdParticion(valor: string) {
    this.idParticion = valor;
  }

  getIdParticion(): string {
    return this.idParticion;
  }

  cerrarSesion() {
    this.sesionActiva = false;
    this.usuario = '';
    this.idParticion = '';
  }
}