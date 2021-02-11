import { Injectable } from '@angular/core';
import { environment } from '../../environments/environment';
import { ActionDto } from '../dto/Actions/actionDto';
import { ActionNames } from '../dto/Actions/actionNames';
import { GameCreatedActionDto } from '../dto/Actions/gameCreatedActionDto';
import { AppState } from '../state/game-state';

@Injectable({
  providedIn: 'root'
})
export class SocketsService {
  socket: WebSocket | undefined;
  url = '';
  constructor() {
    // this.socket = new WebSocket('ws://localhost:60109/ws');
  }

  connect(): void {
    this.url = environment.socketServiceUrl;
    this.socket = new WebSocket(this.url);
    this.socket.onmessage = this.onMessage;
    this.socket.onerror = this.onError;
    this.socket.onopen = this.onOpen;
    this.socket.onclose = this.onClose;
  }

  onOpen(event: Event): void {
    console.log('Open', { event });
  }

  onMessage(message: MessageEvent<ActionDto>): void {
    console.log('Message', message.data);
    console.log('actionName', message.data['actionName']);

    const action = <GameCreatedActionDto>message.data;
    console.log(action);
    AppState.Singleton.game.setValue(action.game);

    // switch (message.data.actionName) {
    //   case ActionNames.gameCreated:
    //     const action = <GameCreatedActionDto>message.data;
    //     console.log('Set game value');
    //     AppState.Singleton.game.setValue(action.game);
    //     break;

    //   default:
    //     break;
    // }
  }

  onError(event: Event): void {
    console.error('Error', { event });
  }

  sendMessage(message: string): void {
    if (this.socket) {
      this.socket.send(message);
    }
  }

  onClose(event: CloseEvent): void {
    console.log('Close', { event });
  }
}
