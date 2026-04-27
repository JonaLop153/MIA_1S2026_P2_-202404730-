import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class BackendService {
  private apiUrl = 'http://localhost:8080';

  constructor(private http: HttpClient) {}

  ejecutarComando(comando: string): Observable<any> {
    return this.http.post(`${this.apiUrl}/comando`, { comando });
  }

  ejecutarScript(script: string): Observable<any> {
    return this.http.post(`${this.apiUrl}/script`, { script });
  }
}