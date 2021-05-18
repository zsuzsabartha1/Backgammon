import { Component } from '@angular/core';
import { FormBuilder, FormGroup, Validators } from '@angular/forms';
import { Router } from '@angular/router';
import { UserDto } from 'src/app/dto';
import { AccountService } from 'src/app/services';
import { AppState } from 'src/app/state/app-state';
import { Busy } from 'src/app/state/busy';
import { Theme } from '../theme/theme';

@Component({
  selector: 'app-account-edit-container',
  templateUrl: './account-edit-container.component.html',
  styleUrls: ['./account-edit-container.component.scss']
})
export class AccountEditContainerComponent {
  constructor(
    private service: AccountService,
    private formBuidler: FormBuilder,
    private router: Router
  ) {
    this.formGroup = this.formBuidler.group({
      name: ['', [Validators.required, Validators.maxLength(100)]],
      emailNotification: [false],
      language: [''],
      theme: ['']
    });

    this.formGroup.get('theme')?.valueChanges.subscribe((theme) => {
      Theme.change(theme);
    });

    AppState.Singleton.user.observe().subscribe((userDto) => {
      this.user = userDto;
      if (userDto) {
        this.formGroup.patchValue({
          name: userDto.name,
          emailNotification: userDto.emailNotification,
          language: userDto.preferredLanguage,
          theme: userDto.theme
        });
      }
    });
  }

  formGroup: FormGroup;
  user: UserDto | null = null;
  confirm = false;

  submit(): void {
    if (this.formGroup.valid) {
      const user = { ...this.user, ...this.formGroup.value };
      Busy.show();
      this.service.saveUser(user).subscribe(() => {
        Busy.hide();
        this.router.navigateByUrl('/lobby');
      });
    }
  }

  cancel(): void {
    this.router.navigateByUrl('/lobby');
  }

  deleteUser(): void {
    this.confirm = true;
  }

  doDeletion(): void {
    this.service.deleteUser();
  }

  nameMissing(): boolean {
    return this.formGroup.get('name')?.errors?.required;
  }
}
