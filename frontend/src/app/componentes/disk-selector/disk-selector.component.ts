import { Component, OnInit } from '@angular/core';
import { BackendService } from '../../servicios/backend.service';

@Component({
  selector: 'app-disk-selector',
  templateUrl: './disk-selector.component.html',
  styleUrls: ['./disk-selector.component.css']
})
export class DiskSelectorComponent implements OnInit {
  discos: any[] = [];
  discoSeleccionado: string = '';

  constructor(private backend: BackendService) {}

  ngOnInit() {
    this.cargarDiscos();
  }

  async cargarDiscos() {
    // Lista de discos conocidos (en producción esto viene del backend)
    this.discos = [
      { nombre: 'Disco1.mia', path: '/home/jona/Proyecto1_MIA/Discos/Disco1.mia', size: '50 MB' },
      { nombre: 'Disco2.mia', path: '/home/jona/Proyecto1_MIA/Discos/Disco2.mia', size: '50 MB' },
      { nombre: 'Disco3.mia', path: '/home/jona/Proyecto1_MIA/Discos/Disco3.mia', size: '13 MB' }
    ];
  }

  seleccionarDisco(disco: any) {
    this.discoSeleccionado = disco.path;
  }
}