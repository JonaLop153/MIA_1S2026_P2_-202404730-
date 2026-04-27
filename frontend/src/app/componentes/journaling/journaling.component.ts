import { Component, OnInit } from '@angular/core';
import { BackendService } from '../../servicios/backend.service';
import { AuthContext } from '../../context/AuthContext';
@Component({
  selector: 'app-journaling',
  templateUrl: './journaling.component.html',
  styleUrls: ['./journaling.component.css']
})
export class JournalingComponent implements OnInit {
  idParticion: string = '';
  journalData: string = '';

  constructor(
    private backend: BackendService,
    private auth: AuthContext
  ) {}

  ngOnInit() {
    this.idParticion = this.auth.getIdParticion() || '';
  }

  verJournal() {
    if (!this.idParticion) {
      this.journalData = 'Debe especificar un ID de partición';
      return;
    }

    const comando = `journaling -id=${this.idParticion}`;
    
    this.backend.ejecutarComando(comando).subscribe({
      next: (respuesta: any) => {
        this.journalData = respuesta.respuesta;
      },
      error: (e) => {
        this.journalData = 'Error al cargar journal';
        console.error(e);
      }
    });
  }
}