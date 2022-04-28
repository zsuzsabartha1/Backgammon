import { Component, EventEmitter, Output } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { MessageType } from 'src/app/dto';
import { MassMailDto } from 'src/app/dto/message';

@Component({
  selector: 'app-mailing',
  templateUrl: './mailing.component.html',
  styleUrls: ['./mailing.component.scss']
})
export class MailingComponent {
  @Output() onSend = new EventEmitter<MassMailDto>();

  form: FormGroup;

  constructor(fb: FormBuilder) {
    this.form = fb.group({ user: [''], pass: [''] });
  }

  send(): void {
    const dto: MassMailDto = {
      password: this.form.get('pass')?.value,
      userName: this.form.get('user')?.value,
      type: MessageType.version36Info
    };
    this.onSend.emit(dto);
  }
}
