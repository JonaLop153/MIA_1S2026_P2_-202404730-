import { Component } from '@angular/core';
import { BackendService } from '../../servicios/backend.service';

@Component({
  selector: 'app-editor',
  templateUrl: './editor.component.html',
  styleUrls: ['./editor.component.css']
})
export class EditorComponent {
  script: string = '';
  ejecutando: boolean = false;

  constructor(private backend: BackendService) {}

  async ejecutarScript() {
    if (!this.script.trim()) return;

    this.ejecutando = true;

    try {
      const respuesta = await this.backend.ejecutarScript(this.script).toPromise();
      alert('Script ejecutado exitosamente');
    } catch (e) {
      alert('Error al ejecutar script');
    }

    this.ejecutando = false;
  }

  cargarArchivo(event: any) {
    const file = event.target.files[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e: any) => {
        this.script = e.target.result;
      };
      reader.readAsText(file);
    }
  }
}