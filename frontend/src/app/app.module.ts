import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { FormsModule } from '@angular/forms';
import { HttpClientModule } from '@angular/common/http';
import { RouterModule, Routes } from '@angular/router';

import { AppComponent } from './app.component';

//  Componentes del Proyecto 1
import { ConsolaComponent } from './componentes/consola/consola.component';
import { ToolbarComponent } from './componentes/toolbar/toolbar.component';
import { EditorComponent } from './componentes/editor/editor.component';

// Componentes Nuevos del Proyecto 2
import { LoginComponent } from './componentes/login/login.component';
import { FileExplorerComponent } from './componentes/file-explorer/file-explorer.component';
import { DiskSelectorComponent } from './componentes/disk-selector/disk-selector.component';
import { PartitionSelectorComponent } from './componentes/partition-selector/partition-selector.component';
import { JournalingComponent } from './componentes/journaling/journaling.component';

const routes: Routes = [
  { path: '', component: AppComponent },
  { path: 'login', component: LoginComponent },
  { path: 'dashboard', component: ConsolaComponent },
  { path: 'explorer', component: FileExplorerComponent },
  { path: 'journaling', component: JournalingComponent }
];

@NgModule({
  declarations: [
    AppComponent,
    ConsolaComponent,
    ToolbarComponent,
    EditorComponent,
    LoginComponent,
    FileExplorerComponent,
    DiskSelectorComponent,
    PartitionSelectorComponent,
    JournalingComponent
  ],
  imports: [
    BrowserModule,
    FormsModule,
    HttpClientModule,
    RouterModule.forRoot(routes)
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }