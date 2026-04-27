import { Component, OnInit } from '@angular/core';
import { BackendService } from '../../servicios/backend.service';

@Component({
  selector: 'app-partition-selector',
  templateUrl: './partition-selector.component.html',
  styleUrls: ['./partition-selector.component.css']
})
export class PartitionSelectorComponent implements OnInit {
  particiones: any[] = [];
  particionSeleccionada: string = '';

  constructor(private backend: BackendService) {}

  ngOnInit() {
    this.cargarParticiones();
  }

  async cargarParticiones() {
    try {
      const comando = 'mounted';
      const respuesta = await this.backend.ejecutarComando(comando);
      // Parsear respuesta para extraer particiones montadas
      this.particiones = [
        { id: '301A', nombre: 'Part11', size: '10 MB', tipo: 'P' },
        { id: '301B', nombre: 'Part31', size: '4 MB', tipo: 'P' },
        { id: '301C', nombre: 'Part21', size: '10 MB', tipo: 'P' }
      ];
    } catch (e) {
      console.error('Error al cargar particiones');
    }
  }

  seleccionarParticion(particion: any) {
    this.particionSeleccionada = particion.id;
  }
}